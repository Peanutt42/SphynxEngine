use std::any::Any;

pub enum Value<'a> {
	Bool(bool),
	I16(i16),
	U16(u16),
	I32(i32),
	U32(u32),
	I64(i64),
	U64(u64),
	I128(i128),
	U128(u128),
	F32(f32),
	F64(f64),
	Str(&'a str),
	String(&'a String),
	Enum(EnumValue),
}

pub enum ValueMut<'a> {
	Bool(&'a mut bool),
	I16(&'a mut i16),
	U16(&'a mut u16),
	I32(&'a mut i32),
	U32(&'a mut u32),
	I64(&'a mut i64),
	U64(&'a mut u64),
	I128(&'a mut i128),
	U128(&'a mut u128),
	F32(&'a mut f32),
	F64(&'a mut f64),
	Str(&'a mut str),
	String(&'a mut String),
	Enum(EnumValueMut<'a>),
}

/// Read-only view of a C-style (fieldless) enum.
#[derive(Clone, Copy, Debug)]
pub struct EnumValue {
	pub type_name: &'static str,
	pub variants: &'static [&'static str],
	pub selected: usize,
}

/// Mutable view of a C-style (fieldless) enum.
pub struct EnumValueMut<'a> {
	pub type_name: &'static str,
	pub variants: &'static [&'static str],
	pub selected: usize,
	set: &'a mut dyn FnMut(usize),
}

impl<'a> EnumValueMut<'a> {
	pub fn new(
		type_name: &'static str,
		variants: &'static [&'static str],
		selected: usize,
		set: &'a mut dyn FnMut(usize),
	) -> Self {
		Self {
			type_name,
			variants,
			selected,
			set,
		}
	}

	/// Selects the variant at `index`. Out-of-range indices are ignored.
	pub fn set(&mut self, index: usize) {
		if index != self.selected && index < self.variants.len() {
			(self.set)(index);
			self.selected = index;
		}
	}
}

pub trait Visitor {
	fn begin_component(&mut self, _type_name: &'static str, _field_count: usize) {}
	fn end_component(&mut self, _type_name: &'static str) {}
	fn begin_struct(&mut self, _type_name: &'static str, _field_count: usize) {}
	fn end_struct(&mut self, _type_name: &'static str) {}
	fn begin_tuple(&mut self, _type_name: &'static str, _len: usize) {}
	fn end_tuple(&mut self, _type_name: &'static str) {}
	fn begin_field(&mut self, _name: &'static str) {}
	fn end_field(&mut self, _name: &'static str) {}
	fn begin_seq(&mut self, _type_name: &'static str, _len: usize) {}
	fn end_seq(&mut self, _type_name: &'static str) {}
	fn visit_value(&mut self, _value: Value<'_>) {}
}

pub trait VisitorMut {
	fn begin_component(&mut self, _type_name: &'static str, _field_count: usize) {}
	fn end_component(&mut self, _type_name: &'static str) {}
	fn begin_struct(&mut self, _type_name: &'static str, _field_count: usize) {}
	fn end_struct(&mut self, _type_name: &'static str) {}
	fn begin_tuple(&mut self, _type_name: &'static str, _len: usize) {}
	fn end_tuple(&mut self, _type_name: &'static str) {}
	fn begin_field(&mut self, _name: &'static str) {}
	fn end_field(&mut self, _name: &'static str) {}
	fn begin_seq(&mut self, _type_name: &'static str, _len: usize) {}
	fn end_seq(&mut self, _type_name: &'static str) {}
	fn visit_value(&mut self, _value: ValueMut<'_>) {}
}

pub trait Reflect: Any {
	fn type_name(&self) -> &'static str {
		std::any::type_name::<Self>()
	}
	fn accept(&self, v: &mut dyn Visitor);
	fn accept_mut(&mut self, v: &mut dyn VisitorMut);

	/// Visits only this value's fields, without emitting callbacks for the value
	/// itself. This is used for top-level components, which are already delimited
	/// by [`Visitor::begin_component`] / [`Visitor::end_component`].
	fn accept_fields(&self, v: &mut dyn Visitor) {
		self.accept(v);
	}
	fn accept_fields_mut(&mut self, v: &mut dyn VisitorMut) {
		self.accept_mut(v);
	}
}

macro_rules! impl_reflect_builtin {
    ($($t:ty => $variant:ident),* $(,)?) => {$(
        impl Reflect for $t {
            fn accept(&self, v: &mut dyn Visitor) { v.visit_value(Value::$variant(*self)); }
            fn accept_mut(&mut self, v: &mut dyn VisitorMut) { v.visit_value(ValueMut::$variant(self)); }
        }
    )*};
}

impl_reflect_builtin! {
	bool => Bool,
	i16 => I16,
	u16 => U16,
	i32 => I32,
	u32 => U32,
	i64 => I64,
	u64 => U64,
	i128 => I128,
	u128 => U128,
	f32 => F32,
	f64 => F64,
}

macro_rules! impl_reflect_tuple {
    ($( ($COUNT:tt: $($T:ident $idx:tt),+) )+) => {$(
        impl<$($T: Reflect),+> Reflect for ($($T,)+) {
            fn accept(&self, v: &mut dyn Visitor) {
				let type_name = self.type_name();
                v.begin_tuple(type_name, $COUNT);
                $(
                    self.$idx.accept(v);
                )+
                v.end_tuple(type_name);
            }

            fn accept_mut(&mut self, v: &mut dyn VisitorMut) {
				let type_name = self.type_name();
                v.begin_tuple(type_name, $COUNT);
                $(
                    self.$idx.accept_mut(v);
                )+
                v.end_tuple(type_name);
            }
        }
    )+};
}
impl_reflect_tuple! {
	(1: A 0)
	(2: A 0, B 1)
	(3: A 0, B 1, C 2)
	(4: A 0, B 1, C 2, D 3)
	(5: A 0, B 1, C 2, D 3, E 4)
	(6: A 0, B 1, C 2, D 3, E 4, F 5)
	(7: A 0, B 1, C 2, D 3, E 4, F 5, G 6)
	(8: A 0, B 1, C 2, D 3, E 4, F 5, G 6, H 7)
}

impl Reflect for String {
	fn accept(&self, v: &mut dyn Visitor) {
		v.visit_value(Value::Str(self.as_str()));
	}
	fn accept_mut(&mut self, v: &mut dyn VisitorMut) {
		v.visit_value(ValueMut::String(self));
	}
}

impl<T: Reflect> Reflect for Vec<T> {
	fn accept(&self, v: &mut dyn Visitor) {
		v.begin_seq(self.type_name(), self.len());
		for item in self {
			item.accept(v);
		}
		v.end_seq(self.type_name());
	}
	fn accept_mut(&mut self, v: &mut dyn VisitorMut) {
		let type_name = self.type_name();
		v.begin_seq(type_name, self.len());
		for item in self {
			item.accept_mut(v);
		}
		v.end_seq(type_name);
	}
}

impl<T: Reflect> Reflect for Option<T> {
	fn accept(&self, v: &mut dyn Visitor) {
		match self {
			Some(item) => {
				v.begin_struct("Some", 1);
				v.begin_field("0");
				item.accept(v);
				v.end_field("0");
				v.end_struct("Some");
			}
			None => {
				v.begin_struct("None", 0);
				v.end_struct("None");
			}
		}
	}
	fn accept_mut(&mut self, v: &mut dyn VisitorMut) {
		match self {
			Some(item) => {
				v.begin_struct("Some", 1);
				v.begin_field("0");
				item.accept_mut(v);
				v.end_field("0");
				v.end_struct("Some");
			}
			None => {
				v.begin_struct("None", 0);
				v.end_struct("None");
			}
		}
	}
}

impl<T: Reflect, const N: usize> Reflect for [T; N] {
	fn accept(&self, v: &mut dyn Visitor) {
		v.begin_seq(std::any::type_name::<Self>(), N);
		for item in self {
			item.accept(v);
		}
		v.end_seq(std::any::type_name::<Self>());
	}
	fn accept_mut(&mut self, v: &mut dyn VisitorMut) {
		v.begin_seq(std::any::type_name::<Self>(), N);
		for item in self {
			item.accept_mut(v);
		}
		v.end_seq(std::any::type_name::<Self>());
	}
}
