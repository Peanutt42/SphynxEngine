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

pub fn log(verbosity: LogVerbosity, topic: LogTopic, formatted: String) {
	let msg = format!("{} {}", topic.to_string(), formatted);
	println!("{} {}", topic.to_string().bright_black(), verbosity.colorize(formatted));

	let log_file_msg = format!("{} {}\n", verbosity.to_string(), msg);

	// log file already created, just write to it
	if let Some(log_file) = &mut *LOG_FILE.lock().unwrap() {
		let _ = log_file.write(log_file_msg.as_bytes());
		return;
	}

	// create new log file, then write to it
	match File::create("Engine.log") {
		Ok(mut log_file) => {
			let _ = log_file.write_all(log_file_msg.as_bytes());
			*LOG_FILE.lock().unwrap() = Some(log_file)
		},
		Err(e) => eprintln!("failed to create engine log (Engine.log): {e}"),
	}
}

#[macro_export]
macro_rules! info {
	($topic:ident, $($arg:tt)*) => {{
		$crate::log($crate::LogVerbosity::Info, $crate::LogTopic::$topic, format!($($arg)*));
	}};
}

#[macro_export]
macro_rules! warning {
	($topic:ident, $($arg:tt)*) => {{
		$crate::log($crate::LogVerbosity::Warning, $crate::LogTopic::$topic, format!($($arg)*));
	}};
}

#[macro_export]
macro_rules! error {
	($topic:ident, $($arg:tt)*) => {{
		$crate::log($crate::LogVerbosity::Error, $crate::LogTopic::$topic, format!($($arg)*));
	}};
}
