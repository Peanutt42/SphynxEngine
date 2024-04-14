

pub struct DepthTexture {
	_texture: wgpu::Texture,
	view: wgpu::TextureView,
	_sampler: wgpu::Sampler,
}

impl DepthTexture {
	pub const FORMAT: wgpu::TextureFormat = wgpu::TextureFormat::Depth32Float;

	pub fn new(device: &wgpu::Device, config: &wgpu::SurfaceConfiguration) -> Self {
		let size = wgpu::Extent3d {
			width: config.width,
			height: config.height,
			depth_or_array_layers: 1,
		};
		let desc = wgpu::TextureDescriptor {
			label: Some("Depth Texture"),
			size,
			mip_level_count: 1,
			sample_count: 1,
			dimension: wgpu::TextureDimension::D2,
			format: Self::FORMAT,
			usage: wgpu::TextureUsages::RENDER_ATTACHMENT | wgpu::TextureUsages::TEXTURE_BINDING,
			view_formats: &[],
		};
		let _texture = device.create_texture(&desc);

		let view = _texture.create_view(&wgpu::TextureViewDescriptor::default());
		let _sampler = device.create_sampler(
			&wgpu::SamplerDescriptor {
				address_mode_u: wgpu::AddressMode::ClampToEdge,
				address_mode_v: wgpu::AddressMode::ClampToEdge,
				address_mode_w: wgpu::AddressMode::ClampToEdge,
				mag_filter: wgpu::FilterMode::Linear,
				min_filter: wgpu::FilterMode::Linear,
				mipmap_filter: wgpu::FilterMode::Nearest,
				compare: Some(wgpu::CompareFunction::LessEqual),
				lod_min_clamp: 0.0,
				lod_max_clamp: 100.0,
				..Default::default()
			}
		);

		Self { _texture, view, _sampler }
	}

	pub fn get_attachment(&self) -> wgpu::RenderPassDepthStencilAttachment {
		wgpu::RenderPassDepthStencilAttachment {
			view: &self.view,
			depth_ops: Some(wgpu::Operations {
				load: wgpu::LoadOp::Clear(1.0),
				store: wgpu::StoreOp::Store,
			}),
			stencil_ops: None,
		}
	}
}
