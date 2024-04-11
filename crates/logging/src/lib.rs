use std::fs::File;
use std::io::Write;
use std::sync::{Arc, Mutex};
use once_cell::sync::Lazy;
use colored::{ColoredString, Colorize};

#[derive(Debug, Copy, Clone)]
pub enum LogTopic {
	General,
	Rendering,
	Audio,
	Networking,
	Scripting,
	Physics,
	UI,
}

impl LogTopic {
	pub fn to_string(self) -> &'static str {
		match self {
			LogTopic::General =>    "[GENERAL]   ",
			LogTopic::Rendering =>  "[RENDERING] ",
			LogTopic::Audio =>      "[AUDIO]     ",
			LogTopic::Networking => "[NETWORKING]",
			LogTopic::Scripting =>  "[SCRIPTING] ",
			LogTopic::Physics =>    "[PHYSICS]   ",
			LogTopic::UI =>         "[UI]        ",
		}
	}
}

#[derive(Debug, Copy, Clone)]
pub enum LogVerbosity {
	Info,
	Warning,
	Error,
}

impl LogVerbosity {
	pub fn to_string(self) -> &'static str {
		match self {
			LogVerbosity::Info =>	 "INFO",
			LogVerbosity::Warning => "WARN",
			LogVerbosity::Error =>	 "ERR ",
		}
	}

	pub fn colorize(self, msg: String) -> ColoredString {
		match self {
			LogVerbosity::Info => msg.bright_white(),
			LogVerbosity::Warning => msg.yellow(),
			LogVerbosity::Error => msg.red(),
		}
	}
}

pub static LOG_FILE: Lazy<Arc<Mutex<Option<File>>>> = Lazy::new(|| Arc::new(Mutex::new(None)));

pub fn init() {
	match File::create("Engine.log") {
		Ok(log_file) => *LOG_FILE.lock().unwrap() = Some(log_file),
		Err(e) => eprintln!("failed to create engine log (Engine.log): {e}"),
	}
}

pub fn write_to_log_file(msg: String) {
	if let Some(log_file) = &mut *LOG_FILE.lock().unwrap() {
		let _ = log_file.write_all(msg.as_bytes());
	}
}

#[macro_export]
macro_rules! log {
	($verbosity:expr, $topic:ident, $($arg:tt)*) => {{
		let formatted = format!("{} {}", $crate::LogTopic::$topic.to_string(), format!($($arg)*));
		println!("{}", $verbosity.colorize(formatted.clone()));
		$crate::write_to_log_file(format!("{} {}\n", $verbosity.to_string(), formatted));
	}};
}

#[macro_export]
macro_rules! info {
	($topic:ident, $($arg:tt)*) => {{
		$crate::log!($crate::LogVerbosity::Info, $topic, $($arg)*);
	}};
}

#[macro_export]
macro_rules! warning {
	($topic:ident, $($arg:tt)*) => {{
		$crate::log!($crate::logging::LogVerbosity::Warning, $topic, $($arg)*);
	}};
}

#[macro_export]
macro_rules! error {
	($topic:ident, $($arg:tt)*) => {{
		$crate::log!($crate::logging::LogVerbosity::Error, $topic, $($arg)*);
	}};
}
