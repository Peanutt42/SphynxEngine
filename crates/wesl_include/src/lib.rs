use proc_macro::TokenStream;
use proc_macro2::TokenStream as TokenStream2;
use quote::quote;
use std::path::{Path, PathBuf};
use std::str::FromStr;
use syn::{
	LitStr, Token,
	parse::{Parse, ParseStream},
	parse_macro_input,
};
use wesl::CompileOptions;
use wesl::resolver::{FileResolver, Router, StandardResolver};
use wesl::syntax::{ModulePath, PathOrigin};
use wesl::toml_cfg::{DependenciesConfig, DependencySpec, WeslToml};

/// `include_wesl!("../../../assets/shaders", "shader_module")` -> &'static str of linked WGSL
#[proc_macro]
pub fn include_wesl(input: TokenStream) -> TokenStream {
	let input2: TokenStream2 = input.clone().into();
	let args = parse_macro_input!(input as Args);
	let shader_dir =
		PathBuf::from(std::env::var("CARGO_MANIFEST_DIR").unwrap()).join(args.dir.value());
	let shader_module_name_str = format!("package::{}", args.module.value());

	expand(&shader_dir, &shader_module_name_str).unwrap_or_else(|e| {
		syn::Error::new_spanned(&input2, format!("WESL compile error: {e}"))
			.to_compile_error()
			.into()
	})
}

fn expand(shader_dir: &Path, shader_module_name_str: &str) -> Result<TokenStream, String> {
	let shader_module_name =
		wesl::syntax::ModulePath::from_str(shader_module_name_str).map_err(|e| e.to_string())?;

	let shader_wesl_config_filepath_str =
		shader_dir.join("wesl.toml").to_string_lossy().to_string();
	let (shader_config, _shader_package_root) =
		load_wesl_package(shader_dir).map_err(|e| e.to_string())?;

	let mut router = Router::new();
	mount_deps(&mut router, None, shader_dir, &shader_config)?;
	// local `package::` files + the virtual `constants` package
	router.mount_fallback_resolver(StandardResolver::new(shader_dir));

	let compiler_options = CompileOptions {
		// visibility is a very early and not stable feature of wesl
		// for example, my neovim lsp does not know about the public keyword etc.
		// TODO: turn on once the ecosystem has stabilized it enough
		visibility: false,
		..Default::default()
	};

	let compiler = wesl::Compiler::new_with_resolver(compiler_options, router);

	let compiler_result = compiler
		.compile_module(&shader_module_name)
		.map_err(|e| e.to_string())?;

	let linked_wesl = compiler_result.syntax.to_string();

	let Some(sourcemap) = compiler_result.sourcemap else {
		return Err("no sourcemap was included by the wesl compiler/linker".to_string());
	};

	// rebuild tracking: depend on every shader file via include_bytes!
	// (stable-compatible alternative to nightly `tracked_path`)
	let linked_files = compiler_result
		.used_items
		.iter()
		.filter_map(|(module_path, _)| {
			if module_path.origin.is_package() || module_path.origin.is_relative() {
				return None;
			}

			let source = sourcemap.file(module_path)?;
			source.path.clone()
		})
		.map(|fp| fp.to_string_lossy().to_string())
		.collect::<Vec<_>>();

	Ok(quote! {
		{
			#( const _: &[u8] = include_bytes!(#linked_files); )*
			const _: &[u8] = include_bytes!(#shader_wesl_config_filepath_str);
			#linked_wesl
		}
	}
	.into())
}

struct Args {
	dir: LitStr,
	module: LitStr,
}

impl Parse for Args {
	fn parse(input: ParseStream) -> syn::Result<Self> {
		let dir = input.parse()?;
		input.parse::<Token![,]>()?;
		let module = input.parse()?;
		let _ = input.parse::<Option<Token![,]>>()?;
		Ok(Args { dir, module })
	}
}

/// returns (config, absolute_shader_root)
fn load_wesl_package(dir: &Path) -> Result<(WeslToml, PathBuf), wesl::error::TomlError> {
	let toml_path = dir.join("wesl.toml");
	let config = WeslToml::from_file(toml_path)?;
	let absolute_shader_root = dir.join(&config.package.root);
	Ok((config, absolute_shader_root))
}

/// mount every `path` dependency of `config` (recursively) onto the router.
fn mount_deps(
	router: &mut Router,
	prefix: Option<&str>,
	shader_package_dir: &Path,
	config: &WeslToml,
) -> Result<(), String> {
	let deps = match &config.dependencies {
		DependenciesConfig::None => return Ok(()),
		DependenciesConfig::Auto => {
			return Err("`dependencies = \"auto\"` is not supported by this macro".into());
		}
		DependenciesConfig::Manual(m) => m,
	};

	for (name, spec) in deps {
		let DependencySpec::Path(rel) = spec else {
			return Err(format!(
				"dependency `{name}`: only `path = \"…\"` dependencies are supported by this macro"
			));
		};
		let dep_dir = shader_package_dir.join(rel);
		let (dep_cfg, dep_root) = load_wesl_package(&dep_dir).map_err(|e| e.to_string())?;

		// sub-dependencies are named `parent/child` by wesl's import resolution
		let full = match prefix {
			Some(p) => format!("{p}/{name}"),
			None => name.clone(),
		};

		router.mount_resolver(
			ModulePath::new(PathOrigin::Package(full.clone()), vec![]),
			FileResolver::new(&dep_root),
		);

		mount_deps(router, Some(&full), &dep_dir, &dep_cfg)?;
	}
	Ok(())
}
