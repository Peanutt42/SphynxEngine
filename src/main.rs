use sphynx_core::EngineConfig;

#[tokio::main]
async fn main() -> anyhow::Result<()> {
	EngineConfig::default().run().await
}
