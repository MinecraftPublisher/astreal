Body = (_ a:(IncludeCall / StructDef / ObjectDefine / Function / FunctionLikeMacro / BasicMacro / FunctionLikeMacroCall / BasicMacroCall) _ { return a })*

IncludeCall = '$' _ 'include' _ '(' _ name:('!'? (!')' .)* { return text() }) _ ')' _ ';'
	{ return { kind: 'include', path: name } }

FunctionLikeMacro = '#' _ 'define' _ replace:Rawword _ args:('(' _ Rawword? _ (',' _ a:Rawword _ { return a })* ')')
	_ _with:((('\\\n' { return '' }) / (!'\n' boo:. { return boo }))*) { return { kind: 'macro-function', replace, with: _with.join('') } }
BasicMacro = '#' _ 'define' _ replace:Rawword _ _with:((('\\\n' { return '' }) / (!'\n' boo:. { return boo }))*) 
	{ return { kind: 'macro-basic', replace, with: _with.join('') } }

FunctionLikeMacroCall = '$' _ name:Rawword content:MacroArgs 
	{ return { kind: 'preprocess-call', name, content: content.substring(1, content.length - 1) } }
MacroArgs = _ '(' (MacroArgs+ / (!'(' !')' .)+)* ')' _ { return text() }
BasicMacroCall = '$' _ name:Rawword { return { kind: 'preprocess-replace', name } }

StructDef = 'struct' _ name:Rawword _ '{' fields:(_ a:VarType _ ';' _ { return a })+ '}'
	{ return { kind: 'type-definition', name, fields } }

ObjectDefine = 'define' _ name:DefineName _ 
	vars:('(' _ VarType? _ (',' _ a:VarType _ { return a })* ')') _ code:Block 
	{ return { kind: 'define-function', name, 
    	vars: vars.map(e => e && e[0]?.kind === 'variable-type' ? e[0] : e)
    	.filter(e => e?.kind === 'variable-type'), code } }

DefineName = main:Rawword parts:(':' _ a:Rawword _ { return a })*
	{ return { kind: 'property', parts: [main, ...parts] } }

Function = type:Rawword _ name:Rawword _ 
	vars:('(' _ VarType? _ (',' _ a:VarType _ { return a })* ')') _ code:Block 
	{ return { kind: 'function', type, name, 
    	vars: vars.map(e => e && e[0]?.kind === 'variable-type' ? e[0] : e)
    	.filter(e => e?.kind === 'variable-type'), code } }

VarType = type:Rawword _ ands:(_ '&' _ { return 1 })* _ name:Rawword 
	{ return { kind: 'variable-type', type, name, dimension: ands.length } }

Block = '{' _ value:(
    		a:(
    			b:(c: Action ';' { return c }) { return b } / 
                b:Block { return b } / 
                b:StateOrLoop { console.log(b); return b }
            ) { return a }
         )* _ '}' 
	{ return { kind: 'block', actions: value } }

Action = _ value:(FunctionLikeMacroCall / BasicMacroCall / FunctionLikeMacro / 
	BasicMacro / ReturnExp / Call / VariableReassignment / VariableAssignment / Block / Expr) _ 
	{ return { kind: 'action', operation: value } }

StateOrLoop = ('if' _ '(' _ e:Expr _ ')' _ code:(Block / Action) { return { kind: 'condition', code, condition: e } }) / 
	('while' _ '(' _ e:Expr _ ')' _ code:(Block / Action) { return { kind: 'while-loop', code, condition: e } }) /
    ('for' _ '(' _ init:VarType _ '=' _ startingValue:Expression _ ';' _ keepGoing:Expr _ ';' _ step:Expr _ ')' _ code:(Block / Action) { return { kind: 'for-loop', code, init: [init, startingValue], condition: keepGoing, cycle: step } })

Expr = start:EvaluatedExpression _ check:(_ a:ExpPartial _ { return a })* { return { kind: 'expr', value: [start, ...check] } }

ExpPart = rest:(_ op:(('&&' / '||' / '==' / '!=' / ([><] '='?) / [<>&|+\-*/%]) 
    _ { return text().trim() }) { return op })

ExpPartial = _ op:ExpPart _ side:EvaluatedExpression+ { return { kind: 'partial-expression', op, side } }

EvaluatedExpression = (op:(('!' / '++' / '--') { return text() }) _ val:EvaluatedExpression { return { kind: 'pre-unary-expression', value: val, op } }) / 
	(val:Expression _ op:(('++' / '--') { return text() }) { return { kind: 'post-unary-expression', value: val, op } }) / 
	(first:Expression _ rest:(_ op:ExpPart _ next:Expression _ { return [ op, next ] })
    { return { kind: 'binary-expression', value: [first, ...rest] } }) /
    Expression

ReturnExp = 'return' _ value:Expression { return { kind: 'return', value } }

VariableReassignment = name:Rawword _  op:AssignmentOperator _ value:Expr _
    { return { kind: 'variable-reassignment', name, value, op } }
VariableAssignment = type:Rawword _ name:Rawword _ '=' _ value:Expr _
	{ return { kind: 'variable-assignment', type, name, value } }
AssignmentOperator = [+\-*/%^&|]? '=' { return text() }

Expression = (cast:Cast _ value:ExpressionPart 
		{ return { kind: 'expression', cast, value } }) / 
    (value:ExpressionPart 
    	{ return { kind: 'expression', value } })
ExpressionPart = a:(Array / Object / Call / Index / Subname / Number / String) { return a }

Call = name:Subname _ calls:(CallPart+) { return { kind: 'call', name, calls } }
CallPart = _ '(' _ arg1:Expression? _ rest:(',' _ value:Expression _ { return value })* ')' _
    { return { kind: 'function-call', args: [arg1, ...rest].filter(e => !!e) } }

Cast = '(' _ to:Rawword ands:(_ '&' _ { return 1 })* _ ')'
	{ return { kind: 'cast', to, dimension: ands.length } }

Index = name:Subname _ vals:('[' _ val:Expression _ ']' { return val })+
	{ return { kind: 'index', indexes: vals } }

Subname = main:Rawword parts:('.' _ a:Rawword _ { return a })*
	{ return { kind: 'property', parts: [main, ...parts] } }

Keyword = value:Rawword { return { kind: 'keyword', value } }
Rawword = [_a-zA-Z][_a-zA-Z$0-9]* { return text().trim() }

Object = '{' _ first:ObjectProp? _ 
	items:(',' _ a:ObjectProp _ { return a })* _ '}'
    {
    	let output = {};
        let input = [first, ...items]
        
        for(let i of input) output[i[0]] = i[1]
        
        return { kind: 'object', values: output }
    }
    
ObjectProp = '.' _ key:Rawword _ '=' _ value:Expression { return [key, value] }

Array = '[' _ first:Expression _ 
	items:(',' _ a:Expression _ { return a })* _ ']' { return { kind: 'array', items: [first, ...items] } }

String = e:(_ StringPart _)+ { return { kind: 'string', value: e.map(f => f[1]).join('') } }
StringPart = '"' text:((!'"' .)*) '"' { return text.map(e => e[1]).join('') }

Number = (RightDecimal / Exponent / NumberPart) 
	{ return { kind: 'number', value: parseFloat(text()) } }
RightDecimal = (base:(Exponent / NumberPart) '.' float:NumberPart 'e' exp:NumberPart) / 
	(base:(Exponent / NumberPart) '.' float:NumberPart)
Exponent = part:NumberPart 'e' exp:NumberPart
NumberPart = '-'? [0-9_]+ 
// HexNumber = '-'? '0x' [0-9A-Fa-f]+ { return text() }

_ "whitespace" = [ \t\n\r]* { return null }