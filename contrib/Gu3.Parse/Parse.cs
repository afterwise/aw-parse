
// Json and text parsing utilities written in 2014-2015 by gumi Sweden.
// No copyright is claimed, and you may use it for any purpose you like.
// No warranty for any purpose is expressed or implied by the authors.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;

namespace Gu3 {

public enum ParseToken {
	Stop,
	Pop,

	Brace,
	Bracket,
	Paren,

	Comma,

	Let,
	Sym,
	Str,

	Int,
	Float,

	Any
}

[StructLayout(LayoutKind.Explicit)]
public unsafe struct ParseValue {
	[FieldOffset(0)] public sbyte *s;
	[FieldOffset(0)] public long i;
	[FieldOffset(0)] public double f;

	public float ToFloat(ParseToken pt) {
		return (pt == ParseToken.Int) ? (float) i :
			(pt == ParseToken.Float) ? (float) f :
			(pt == ParseToken.Let || pt == ParseToken.Sym || pt == ParseToken.Str) &&
					s != null && *s != 0 ?
				float.Parse(new string(s), CultureInfo.InvariantCulture) : 0f;
	}

	public double ToDouble(ParseToken pt) {
		return (pt == ParseToken.Int) ? (double) i :
			(pt == ParseToken.Float) ? f :
			(pt == ParseToken.Let || pt == ParseToken.Sym || pt == ParseToken.Str) &&
					s != null && *s != 0 ?
				double.Parse(new string(s), CultureInfo.InvariantCulture) : 0.0;
	}

	public byte ToByte(ParseToken pt) {
		return (byte) ToInt(pt);
	}

	public short ToShort(ParseToken pt) {
		return (short) ToInt(pt);
	}

	public int ToInt(ParseToken pt) {
		return (pt == ParseToken.Int) ? (int) i :
			(pt == ParseToken.Float) ? (int) f :
			(pt == ParseToken.Let || pt == ParseToken.Sym || pt == ParseToken.Str) &&
					s != null && *s != 0 ?
				int.Parse(new string(s), CultureInfo.InvariantCulture) : 0;
	}

	public long ToLong(ParseToken pt) {
		return (pt == ParseToken.Int) ? i :
			(pt == ParseToken.Float) ? (long) f :
			(pt == ParseToken.Let || pt == ParseToken.Sym || pt == ParseToken.Str) &&
					s != null && *s != 0 ?
				long.Parse(new string(s), CultureInfo.InvariantCulture) : 0;
	}

	public string ToString(ParseToken pt) {
		return (pt == ParseToken.Int) ? i.ToString(CultureInfo.InvariantCulture) :
			(pt == ParseToken.Float) ? f.ToString(CultureInfo.InvariantCulture) :
			(pt == ParseToken.Let || pt == ParseToken.Sym || pt == ParseToken.Str) &&
					s != null && *s != 0 ?
				new string(s) : "";
	}

	public bool ToBool(ParseToken pt) {
		return (pt == ParseToken.Int) ? (i != 0) :
			(pt == ParseToken.Float) ? (f != 0.0) :
			(pt == ParseToken.Let || pt == ParseToken.Sym || pt == ParseToken.Str) && *s != 0 ?
				bool.Parse(new string(s)) : false;
	}
}

public struct ParseNode {
	public ParseToken token;
	public object value;
}

public static class Parse {
	static unsafe ParseToken Term(Stack<ParseNode> stack, sbyte *ptr, sbyte **end) {
		List<ParseNode> list;
		ParseValue pv;
		ParseToken pt = Token(out pv, ptr, &ptr);

		switch (pt) {
		case ParseToken.Stop:
		case ParseToken.Pop:
			break;

		case ParseToken.Brace:
		case ParseToken.Bracket:
		case ParseToken.Paren:
			list = new List<ParseNode>();
			List(list, stack, ptr, &ptr);
			stack.Push(new ParseNode() {token = pt, value = list});
			break;

		case ParseToken.Comma:
			stack.Push(new ParseNode() {token = pt, value = null});
			break;

		case ParseToken.Let:
		case ParseToken.Sym:
		case ParseToken.Str:
			stack.Push(new ParseNode() {token = pt, value = new string(pv.s)});
			break;

		case ParseToken.Int:
			stack.Push(new ParseNode() {token = pt, value = pv.i});
			break;
	
		case ParseToken.Float:
			stack.Push(new ParseNode() {token = pt, value = pv.f});
			break;
		}

		if (end != null)
			*end = ptr;

		return pt;
	}

	static unsafe ParseToken List(List<ParseNode> list, Stack<ParseNode> stack, sbyte *ptr, sbyte **end) {
		ParseToken pt;

		while ((pt = Term(stack, ptr, &ptr)) > ParseToken.Pop)
			list.Add(stack.Pop());

		if (end != null)
			*end = ptr;

		return pt;
	}

	public static unsafe List<ParseNode> Tree(string s) {
		var list = new List<ParseNode>();
		var stack = new Stack<ParseNode>();

		Text(s, p => List(list, stack, p, null));
		return list;
	}

	public unsafe delegate void ParseMethod(sbyte *p);

	public static unsafe void Text(string s, ParseMethod m) {
		sbyte *buf = null;

		try {
			if ((buf = AllocBuffer(s)) != null)
				m(buf);
		} finally {
			if (buf != null)
				FreeBuffer(buf);
		}
	}

	public delegate void ParseCsvMethod(ParseCsv csv);

	public static unsafe void CsvText(string s, ParseCsvMethod m) {
		Text(s, p => {
			var csv = new ParseCsv(p);
			csv.ReadHeader();
			m(csv);
		});
	}

	public static unsafe void CsvText(string s, Action<string, ParseCsv> a) {
		Text(s, p => {
			var csv = new ParseCsv(p);
			csv.ReadHeader();
			csv.header.ForEach(x => a(x, csv));
		});
	}

	public static unsafe sbyte *AllocBuffer(string str) {
		return (sbyte *) Marshal.StringToHGlobalAnsi(str);
	}

	public static unsafe void FreeBuffer(sbyte *ptr) {
		if (ptr != null)
			Marshal.FreeHGlobal((System.IntPtr) ptr);
	}

#if (UNITY_IPHONE || UNITY_XBOX360) && !UNITY_EDITOR
	[DllImport("__Internal", EntryPoint = "parse_token")]
#else
	[DllImport("aw-parse", EntryPoint = "parse_token")]
#endif
	static extern unsafe ParseToken parse_token(out ParseValue pv, sbyte *str, sbyte **end);

	public static unsafe ParseToken Token(out ParseValue pv, sbyte *str, sbyte **end) {
		return parse_token(out pv, str, end);
	}

#if (UNITY_IPHONE || UNITY_XBOX360) && !UNITY_EDITOR
	[DllImport("__Internal", EntryPoint = "parse_skip_token")]
#else
	[DllImport("aw-parse", EntryPoint = "parse_skip_token")]
#endif
	static extern unsafe void parse_skip_token(ParseToken pt, sbyte *str, sbyte **end);

	public static unsafe void SkipToken(ParseToken pt, sbyte *str, sbyte **end) {
		parse_skip_token(pt, str, end);
	}

#if (UNITY_IPHONE || UNITY_XBOX360) && !UNITY_EDITOR
	[DllImport("__Internal", EntryPoint = "parse_skip_to_end")]
#else
	[DllImport("aw-parse", EntryPoint = "parse_skip_to_end")]
#endif
	static extern unsafe uint parse_skip_to_end(ParseToken pt, sbyte *str, sbyte **end);

	public static unsafe uint SkipToEnd(ParseToken pt, sbyte *str, sbyte **end) {
		return parse_skip_to_end(pt, str, end);
	}
}

public class Json {
	StringBuilder _buf;
	ulong _mask; // 64 levels or bust

	public Json() {
		_buf = new StringBuilder(2048);
	}

	public Json Clear() {
		_buf.Length = 0;
		_mask = 0;
		return this;
	}

	void Begin(char c) {
		VanillaPrefix();
		_buf.Append(c);
		_mask <<= 1;
	}

	void End(char c) {
		_buf.Append(c);
		_mask = (_mask >> 1) | 1;
	}

	public Json BeginObject() {
		Begin('{');
		return this;
	}

	public Json EndObject() {
		End('}');
		return this;
	}

	public Json BeginArray() {
		Begin('[');
		return this;
	}

	public Json EndArray() {
		End(']');
		return this;
	}

	void StringPrefix() {
		if ((_mask & 1) != 0)
			_buf.Append(",\"");
		else
			_buf.Append('"');
	}

	void VanillaPrefix() {
		if ((_mask & 1) != 0)
			_buf.Append(",");
	}

	public Json Name(string name) {
		StringPrefix();
		_buf.Append(name);
		_buf.Append("\":");
		_mask &= ~((ulong) 1);
		return this;
	}

	public Json Value(bool value) {
		VanillaPrefix();
		_buf.Append(value ? "true" : "false");
		_mask |= 1;
		return this;
	}

	public Json Value(long value) {
		VanillaPrefix();
		_buf.Append(value);
		_mask |= 1;
		return this;
	}

	public Json Value(double value) {
		VanillaPrefix();
		_buf.AppendFormat(CultureInfo.InvariantCulture, "{0:R}", value);
		_mask |= 1;
		return this;
	}

	public Json Value(string value) {
		StringPrefix();
		_buf.Append(value);
		_buf.Append('"');
		_mask |= 1;
		return this;
	}

	public static unsafe string Escape(string str) {
		int i, n = str.Length;
		char *buf = stackalloc char[n * 2 + 1];
		char *ptr = buf;

		for (i = 0; i < n; ++i) {
			char c = str[i];
			switch (c) {
			case '"':
				*ptr++ = '\\';
				*ptr++ = '"';
				break;
			case '\\':
				*ptr++ = '\\';
				*ptr++ = '\\';
				break;
			case '\b':
				*ptr++ = '\\';
				*ptr++ = 'b';
				break;
			case '\f':
				*ptr++ = '\\';
				*ptr++ = 'f';
				break;
			case '\r':
				*ptr++ = '\\';
				*ptr++ = 'r';
				break;
			case '\n':
				*ptr++ = '\\';
				*ptr++ = 'n';
				break;
			case '\t':
				*ptr++ = '\\';
				*ptr++ = 't';
				break;
			default:
				*ptr++ = c;
				break;
			}
		}

		*ptr++ = '\0';
		return new string(buf);
	}

	public Json EscapedValue(string value) {
		return Value(Escape(value));
	}

	public Json Value(Json j) {
		VanillaPrefix();
		_buf.Append(j.ToString());
		_mask |= 1;
		return this;
	}

	public Json Value(object o) {
		if (o is float || o is double)
			Value(Convert.ToDouble(o));
		else if (o is int || o is long)
			Value(Convert.ToInt64(o));
		else if (o is bool)
			Value((bool) o);
		else if (o is string)
			Value((string) o);
		else
			Null();
		return this;
	}

	public Json Verbatim(string s) {
		VanillaPrefix();
		_buf.Append(s);
		_mask |= 1;
		return this;
	}

	public Json Null() {
		VanillaPrefix();
		_buf.Append("null");
		_mask |= 1;
		return this;
	}

	public override string ToString() {
		return _buf.ToString();
	}
}

public enum ParseCsvResult {
	EndOfFile,
	Record,
	EndOfRecord
}

public unsafe class ParseCsv {
	public sbyte *ptr;
	public List<string> header;
	public ParseToken token;
	public ParseValue value;
	public ParseCsvResult result;

	public bool done { get { return result == ParseCsvResult.EndOfFile; }}

	public ParseCsv(sbyte *ptr) {
		this.ptr = ptr;
		header = null;
		token = ParseToken.Stop;
		value = default(ParseValue);
		result = ParseCsvResult.EndOfFile;
	}

	public void ReadHeader() {
		header = new List<string>(16);
		do header.Add(ReadString());
		while (result == ParseCsvResult.Record);
	}

	public void ReadAll(Action<string> a) {
		header.ForEach(a);
	}

	public void Read() {
		var start = ptr;

		result = ParseCsvResult.EndOfFile;
		token = ParseToken.Stop;
		value = default(ParseValue);

		while (!(*ptr == ',' || *ptr == '\n' || *ptr == '\r' || *ptr == 0))
			++ptr;

		if (*ptr == '\r' && ptr[1] == '\n')
			*ptr++ = 0;

		if (*ptr == ',')
			result = ParseCsvResult.Record;
		else if (*ptr == '\n' || *ptr == '\r')
			result = ParseCsvResult.EndOfRecord;
		else if (*ptr == 0)
			result = ParseCsvResult.EndOfFile;

		if (*start == '"' || (*start >= '0' && *start <= '9')) {
			token = Parse.Token(out value, start, &start);
			if (ptr < start)
				ptr = start;
		} else {
			token = ParseToken.Str;
			value.s = start;
			*ptr = 0;
		}

		if (result != ParseCsvResult.EndOfFile && *++ptr == 0)
			result = ParseCsvResult.EndOfFile;
	}

	public float ReadFloat() { Read(); return value.ToFloat(token); }
	public double ReadDouble() { Read(); return value.ToDouble(token); }
	public byte ReadByte() { Read(); return value.ToByte(token); }
	public short ReadShort() { Read(); return value.ToShort(token); }
	public int ReadInt() { Read(); return value.ToInt(token); }
	public long ReadLong() { Read(); return value.ToLong(token); }
	public string ReadString() { Read(); return value.ToString(token); }
	public bool ReadBool() { Read(); return value.ToBool(token); }

	public void Read(out float v) { v = ReadFloat(); }
	public void Read(out double v) { v = ReadDouble(); }
	public void Read(out byte v) { v = ReadByte(); }
	public void Read(out short v) { v = ReadShort(); }
	public void Read(out int v) { v = ReadInt(); }
	public void Read(out long v) { v = ReadLong(); }
	public void Read(out string v) { v = ReadString(); }
	public void Read(out bool v) { v = ReadBool(); }
}

public class Csv {
	StringBuilder _buf;
	int _num;

	public Csv() {
		_buf = new StringBuilder(2048);
	}

	public Csv Clear() {
		_buf.Length = 0;
		_num = 0;
		return this;
	}

	public Csv BeginRecord() {
		if (_num > 0)
			_buf.Append('\n');
		_num = 0;
		return this;
	}

	void StringPrefix() {
		if (_num > 0)
			_buf.Append(",\"");
		else
			_buf.Append('"');
	}

	void VanillaPrefix() {
		if (_num != 0)
			_buf.Append(",");
	}

	public Csv Value(long value) {
		VanillaPrefix();
		_buf.Append(value);
		++_num;
		return this;
	}

	public Csv Value(double value) {
		VanillaPrefix();
		_buf.AppendFormat(CultureInfo.InvariantCulture, "{0:R}", value);
		++_num;
		return this;
	}

	public Csv Value(string value) {
		if (string.IsNullOrEmpty(value)) {
			VanillaPrefix();
		} else {
			StringPrefix();
			_buf.Append(value);
			_buf.Append('"');
		}
		++_num;
		return this;
	}

	public Csv Value(bool value) {
		VanillaPrefix();
		_buf.Append(value);
		++_num;
		return this;
	}

	public Csv Value(object o) {
		if (o is float || o is double)
			Value(Convert.ToDouble(o));
		else if (o is byte || o is short || o is int || o is long)
			Value(Convert.ToInt64(o));
		else if (o is string)
			Value((string) o);
		else if (o == null)
			Value("");
		else if (o is bool)
			Value((bool) o);
		else
			Debug.LogWarning("Unsupported type: " + o.GetType());
		return this;
	}

	public Csv Values(object[,] values) {
		for (int i = 0; i <= values.GetUpperBound(0); ++i) {
			BeginRecord();
			for (int j = 0; j <= values.GetUpperBound(1); ++j)
				Value(values[i, j]);
		}

		return this;
	}

	public Csv Transpose(object[,] values) {
		for (int j = 0; j <= values.GetUpperBound(1); ++j) {
			BeginRecord();
			for (int i = 0; i <= values.GetUpperBound(0); ++i)
				Value(values[i, j]);
		}

		return this;
	}

	public override string ToString() {
		return _buf.ToString();
	}
}

} // namespace Gu3

