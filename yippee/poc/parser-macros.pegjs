Body = (_ a:(IncludeCall / StructDef / ObjectDefine / Function / FunctionLikeMacro / BasicMacro / FunctionLikeMacroCall / Keyword) _ { return a })*

IncludeCall = 'include' _ target:(('[' _ name:('!'? (!')' .)* { return text() }) _ ']' { return { name, is_path: false } }) / (('!'? (!')' .)* { return { name: text(), is_path: true } })) _ ';'
	{ return { kind: 'include', path: target.name, is_path: target.is_path, text: text() } }

FunctionLikeMacro = 'macro' _ replace:Rawword _ args:('(' _ word1:Rawword? _ rest:(',' _ a:Rawword _ { return a })* abyss:(_ ',' _ '...' _ { return 1n })? ')' { return [word1, ...rest, abyss] })
	_ _with:Pooka { return { kind: 'macro-function', replace, args: args.filter(e => e), with: _with.substring(1, _with.length - 1), text: text() } }
Pooka = (('{' ((!'}' !'{' .) / ('{' Pooka '}'))* '}'))* { return text() }
BasicMacro = '#' _ 'define' _ replace:Rawword _ _with:((('\\\n' { return '' }) / (!'\n' boo:. { return boo }))*) 
	{ return { kind: 'macro-basic', replace, with: _with.join(''), text: text() } }

FunctionLikeMacroCall = '$' _ name:Rawword content:MacroArgs 
	{ return { kind: 'preprocess-call', name, content: content, text: text() } }
MacroArgs = _ '(' a:Arg1? args:(',' b:Arg1 { return b })* ')' _ { return [a, ...args] }
Arg1 = a:(![(),] .)* { return text() }

StructDef = 'struct' _ name:Rawword _ '{' fields:(_ a:VarType _ ';' _ { return a })+ '}'
	{ return { kind: 'type-definition', name, fields, text: text() } }

ObjectDefine = 'define' _ '(' _ return:Rawword _ ')' _ name:DefineName _ 
	vars:('(' _ VarType? _ (',' _ a:VarType _ { return a })* ')') _ code:Block 
	{ return { kind: 'define-function', name, return: return,
    	vars: vars.map(e => e && e[0]?.kind === 'variable-type' ? e[0] : e)
    	.filter(e => e?.kind === 'variable-type'), code, text: text() } }

DefineName = main:Rawword parts:(':' _ a:Rawword _ { return a })*
	{ return { kind: 'property', parts: [main, ...parts], text: text() } }

Function = (! 'macro') type:Rawword _ name:Rawword _ 
	vars:('(' _ VarType? _ (',' _ a:VarType _ { return a })* ')') _ code:Block 
	{ return { kind: 'function', type, name, 
    	vars: vars.map(e => e && e[0]?.kind === 'variable-type' ? e[0] : e)
    	.filter(e => e?.kind === 'variable-type'), code, text: text() } }

VarType = type:Rawword _ ands:(_ '&' _ { return 1 })* _ name:Rawword 
	{ return { kind: 'variable-type', type, name, dimension: ands.length, text: text() } }

Block = '{' _ value:(_ g:(
    		a:(
				b:StateOrLoop { return b } /
                b:(Block / FunctionLikeMacro / BasicMacro) { return b } /
    			b:(c: Action ';' { return c }) { return b }
            ) { return a }
         ) _ { return g })* _ '}' 
	{ return { kind: 'block', actions: value, text: text() } }

Action = _ value:(FunctionLikeMacroCall / ReturnExp / (!'if' !'while' !'for' f:Call { return f }) / VariableReassignment / VariableAssignment / Block / Keyword) _ 
	{ return { kind: 'action', operation: value, text: text() } }

StateOrLoop = ('if' _ '(' _ e:Expression _ ')' _ code:(Block / Action) { return { kind: 'condition', condition: e, code, text: text() } }) / 
	('while' _ '(' _ e:Expression _ ')' _ code:(Block / Action) { return { kind: 'while-loop', condition: e, code, text: text() } }) /
    ('for' _ '(' _ start:Variable _ ';' _ keepGoing:Expression _ ';' _ step:Expression _ ')' _ code:(Block / Action) { return { kind: 'for-loop', start, condition: keepGoing, cycle: step, code, text: text() } })

ReturnExp = 'return' _ value:Expression { return { kind: 'return', value, text: text() } }

Variable = VariableAssignment / VariableReassignment
VariableReassignment = name:Rawword _  op:AssignmentOperator _ value:Expression _
    { return { kind: 'variable-reassignment', name, value, op, text: text() } }
VariableAssignment = type:Rawword _ name:Rawword _ '=' _ value:Expression _
	{ return { kind: 'variable-assignment', type, name, value, text: text() } }
AssignmentOperator = [+\-*/%^&|]? '=' { return text() }

Expression = (cast:Cast _ value:ExpressionPart 
		{ return { kind: 'expression', cast, value, text: text(), text: text() } }) / 
    (value:ExpressionPart 
    	{ return { kind: 'expression', value, text: text(), text: text() } })
ExpressionPart = a:(Array / Object / Call / Index / Subname / Number / String) { return a }

Call = name:SubnameCritical _ calls:(CallPart+) { return { kind: 'call', name, calls, text: text() } }
CallPart = _ '(' _ arg1:Expression? _ rest:(',' _ value:Expression _ { return value })* ')' _
    { return { kind: 'function-call', args: [arg1, ...rest].filter(e => !!e), text: text() } }

Cast = '(' _ to:Rawword ands:(_ '&' _ { return 1 })* _ ')'
	{ return { kind: 'cast', to, dimension: ands.length, text: text() } }

Index = name:Subname _ vals:('[' _ val:Expression _ ']' { return val })+
	{ return { kind: 'index', indexes: vals, text: text() } }

SubnameCritical = main:RawwordCritical parts:('.' _ a:Rawword _ { return a })*
	{ return { kind: 'property', parts: [main, ...parts], text: text() } }

Subname = main:Rawword parts:('.' _ a:Rawword _ { return a })*
	{ return { kind: 'property', parts: [main, ...parts], text: text() } }

Keyword = value:Rawword { return { kind: 'keyword', value, text: text() } }
Rawword = [_a-zA-Z][_a-zA-Z$0-9]* { return text().trim() }
RawwordCritical = [_a-zA-Z<>&|+\-*%/=][_a-zA-Z$0-9<>&|+\-*%/=]* { return text().trim() }

Object = '{' _ first:ObjectProp? _ 
	items:(',' _ a:ObjectProp _ { return a })* _ '}'
    {
    	let output = {};
        let input = [first, ...items]
        
        for(let i of input) output[i[0]] = i[1]
        
        return { kind: 'object', values: output, text: text() }
    }
    
ObjectProp = '.' _ key:Rawword _ '=' _ value:Expression { return [key, value] }

Array = '[' _ first:Expression _ 
	items:(',' _ a:Expression _ { return a })* _ ']' { return { kind: 'array', items: [first, ...items], text: text() } }

String = e:(_ StringPart _)+ { return { kind: 'string', value: e.map(f => f[1]).join(''), text: text() } }
StringPart = '"' text:((!'"' .)*) '"' { return text.map(e => e[1]).join('') }

Number = (RightDecimal / Exponent / NumberPart) 
	{ return { kind: 'number', value: parseFloat(text()), text: text() } }
RightDecimal = (base:(Exponent / NumberPart) '.' float:NumberPart 'e' exp:NumberPart) / 
	(base:(Exponent / NumberPart) '.' float:NumberPart)
Exponent = part:NumberPart 'e' exp:NumberPart
NumberPart = '-'? [0-9_]+ 
// HexNumber = '-'? '0x' [0-9A-Fa-f]+ { return text() }

_ "whitespace" = ([ \t\n\r] / _c1 / _c2)* { return null }
_c1 "multi-line comment" = '/*' (!'*/' .)* '*/' { return null }
_c2 "single-line comment" = '//' (!'\n' .)* '\n' { return null }