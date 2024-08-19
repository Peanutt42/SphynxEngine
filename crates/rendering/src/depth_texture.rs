use wgpu::{Texture, TextureView, TextureViewDescriptor, TextureFormat, TextureUsages, TextureDimension, TextureDescriptor, FilterMode, Device, Sampler, SamplerDescriptor, SurfaceConfiguration, Extent3d, AddressMode, CompareFunction, RenderPassDepthStencilAttachment, Operations, LoadOp, StoreOp};

pub struct DepthTexture {
	_texture: Texture,
	view: TextureView,
	_sampler: Sampler,
}

impl DepthTexture {
	pub const FORMAT: TextureFormat = TextureFormat::Depth32Float;

	pub fn new(device: &Device, config: &SurfaceConfiguration) -> Self {
		let size = Extent3d {
			width: config.width,
			height: config.height,
			depth_or_array_layers: 1,
		};
		let desc = TextureDescriptor {
			label: Some("Depth Texture"),
			size,
			mip_level_count: 1,
			sample_count: 1,
			dimension: TextureDimension::D2,
			format: Self::FORMAT,
			usage: TextureUsages::RENDER_ATTACHMENT | TextureUsages::TEXTURE_BINDING,
			view_formats: &[],
		};
		let _texture = device.create_texture(&desc);

		let view = _texture.create_view(&TextureViewDescriptor::default());
		let _sampler = device.create_sampler(
			&SamplerDescriptor {
				address_mode_u: AddressMode::ClampToEdge,
				address_mode_v: AddressMode::ClampToEdge,
				address_mode_w: AddressMode::ClampToEdge,
				mag_filter: FilterMode::Linear,
				min_filter: FilterMode::Linear,
				mipmap_filter: FilterMode::Nearest,
				compare: Some(CompareFunction::LessEqual),
				lod_min_clamp: 0.0,
				lod_max_clamp: 100.0,
				..Default::default()
			}
		);

		Self { _texture, view, _sampler }
	}

	pub fn get_attachment(&self) -> RenderPassDepthStencilAttachment {
		RenderPassDepthStencilAttachment {
			view: &self.view,
			depth_ops: Some(Operations {
				load: LoadOp::Clear(1.0),
				store: StoreOp::Store,
			}),
			stencil_ops: None,
		}
	}
}
