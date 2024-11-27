Body "code" = (_ a:(IncludeCall / Function / StructDef / ObjectDefine / Keyword) _ { return a })*

IncludeCall "import directive" = 'import' _ target:(('[' _ name:((!']' .)+ { return text() }) _ ']' { return { name, is_path: false } }) / ((![\[\]]) (!';' .)+ { return { name: text(), is_path: true } })) _ ';'
	{ return { kind: 'include', path: target.name, is_path: target.is_path, text: text() } }

StructDef "type definition" = 'struct' _ name:Rawword _ '{' fields:(_ a:(FunctionHeader / VarType) _ ';' _ { return a })+ '}'
	{ return { kind: 'type-definition', name, fields, text: text() } }

ObjectDefine "object definition" = 'define' _ name:DefineName _ 
	vars:('(' _ VarType? _ (',' _ a:VarType _ { return a })* ')') _ code:Block 
	{ return { kind: 'define-function', name, 
    	vars: vars.map(e => e && e[0]?.kind === 'variable-type' ? e[0] : e)
    	.filter(e => e?.kind === 'variable-type'), code, text: text() } }

DefineName "object function name" = main:Rawword parts:(':' _ a:Rawword _ { return a })*
	{ return { kind: 'property', parts: [main, ...parts], text: text() } }

Function "function definition" = header:FunctionHeader _ code:Block 
	{ return { kind: 'function', ...header, code, text: text() } }
FunctionHeader "function header" = (! 'macro') (! 'define') (! 'struct') (! 'import') type:Rawword _ name:Rawword _ 
	vars:('(' _ VarType? _ (',' _ a:VarType _ { return a })* ')') { return { type, name, 
    	vars: vars.map(e => e && e[0]?.kind === 'variable-type' ? e[0] : e)
    	.filter(e => e?.kind === 'variable-type') } }

VarType "variable type" = type:Rawword _ ands:(_ '&' _ { return 1 })* _ name:Rawword 
	{ return { kind: 'variable-type', type, name, dimension: ands.length, text: text() } }

Block "code block" = '{' _ value:(_ g:(
    		a:(
				b:StateOrLoop { return b } /
                b:(Block) { return b } /
    			b:(c: Action ';' { return c }) { return b }
            ) { return a }
         ) _ { return g })* _ '}' 
	{ return { kind: 'block', actions: value, text: text() } }

Action "code action" = _ value:(ReturnExp / (!'if' !'while' !'for' f:Call { return f }) / VariableReassignment / VariableAssignment / Block / Keyword) _ 
	{ return { kind: 'action', operation: value, text: text() } }

StateOrLoop "if statement, while or for loop" = ('if' _ '(' _ e:Expression _ ')' _ code:(Block / Action) { return { kind: 'condition', condition: e, code, text: text() } }) / 
	('while' _ '(' _ e:Expression _ ')' _ code:(Block / Action) { return { kind: 'while-loop', condition: e, code, text: text() } }) /
    ('for' _ '(' _ start:Variable _ ';' _ keepGoing:Expression _ ';' _ step:(VariableReassignment / Expression) _ ')' _ code:(Block / Action) { return { kind: 'for-loop', start, condition: keepGoing, cycle: step, code, text: text() } })

ReturnExp "return expression" = 'return' _ value:Expression { return { kind: 'return', value, text: text() } }

Variable "variable declaration or assignment" = VariableAssignment / VariableReassignment
VariableReassignment "variable reassignment" = name:Subname _  op:AssignmentOperator _ value:Expression _
    { return { kind: 'variable-reassignment', name, value, op, text: text() } }
VariableAssignment "variable assignment" = type:Rawword _ name:Rawword _ '=' _ value:Expression _
	{ return { kind: 'variable-assignment', type, name, value, text: text() } }
AssignmentOperator "assignment operator" = [+\-*/%^&|]? '=' { return text() }

Expression "expression" = (cast:Cast _ value:ExpressionPart 
		{ return { kind: 'expression', cast, value, text: text(), text: text() } }) / 
    (value:ExpressionPart 
    	{ return { kind: 'expression', value, text: text(), text: text() } })
ExpressionPart = a:(Array / Object / Call / Index / Subname / Number / String) { return a }

Call "function call" = name:SubnameCritical _ calls:(CallPart+) { return { kind: 'call', name, calls, text: text() } }
CallPart = _ '(' _ arg1:Expression? _ rest:(',' _ value:Expression _ { return value })* ')' _
    { return { kind: 'function-call', args: [arg1, ...rest].filter(e => !!e), text: text() } }

Cast "type casting" = '(' _ to:Rawword ands:(_ '&' _ { return 1 })* _ ')'
	{ return { kind: 'cast', to, dimension: ands.length, text: text() } }

Index "array index" = name:Subname _ vals:('[' _ val:Expression _ ']' { return val })+
	{ return { kind: 'index', indexes: vals, text: text() } }

SubnameCritical = main:RawwordCritical parts:('.' _ a:Rawword _ { return a })*
	{ return { kind: 'property', parts: [main, ...parts], text: text() } }

Subname "loose keyword" = main:Rawword parts:('.' _ a:Rawword _ { return a })*
	{ return { kind: 'property', parts: [main, ...parts], text: text() } }

Keyword "keyword" = value:Rawword { return { kind: 'keyword', value, text: text() } }
Rawword = [_a-zA-Z][_a-zA-Z$0-9]* { return text().trim() }
RawwordCritical = [_a-zA-Z<>&|+\-*%/=][_a-zA-Z$0-9<>&|+\-*%/=]* { return text().trim() }

Object "object instance" = '{' _ first:ObjectProp? _ 
	items:(',' _ a:ObjectProp _ { return a })* _ '}'
    {
    	let output = {};
        let input = [first, ...items]
        
        for(let i of input) output[i[0]] = i[1]
        
        return { kind: 'object', values: output, text: text() }
    }
    
ObjectProp "object property" = '.' _ key:Rawword _ '=' _ value:Expression { return [key, value] }

Array "array instance" = '[' _ first:Expression _ 
	items:(',' _ a:Expression _ { return a })* _ ']' { return { kind: 'array', items: [first, ...items], text: text() } }

String "string" = e:(_ StringPart _)+ { return { kind: 'string', value: e.map(f => f[1]).join(''), text: text() } }
StringPart = '"' text:((!'"' .)*) '"' { return text.map(e => e[1]).join('') }

Number "number" = (RightDecimal / Exponent / NumberPart) 
	{ return { kind: 'number', value: parseFloat(text()), text: text() } }
RightDecimal = (base:(Exponent / NumberPart) '.' float:NumberPart 'e' exp:NumberPart) / 
	(base:(Exponent / NumberPart) '.' float:NumberPart)
Exponent = part:NumberPart 'e' exp:NumberPart
NumberPart = '-'? [0-9_]+ 
// HexNumber = '-'? '0x' [0-9A-Fa-f]+ { return text() }

_ "whitespace" = ([ \t\n\r] / _c1 / _c2)* { return null }
_c1 "multi-line comment" = '/*' (!'*/' .)* '*/' { return null }
_c2 "single-line comment" = '//' (!'\n' .)* '\n' { return null }