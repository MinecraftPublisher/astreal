type Main = (IncludeCall | StructDef | ObjectDefine | VariableAssignment | _Function)[]
type IncludeCall = { kind: 'include', path: string, is_path: boolean } & NodeData
type StructDef = { kind: 'type-definition', name: string, fields: (VarType | FunctionHeader)[] } & NodeData
type VarType = { kind: 'variable-type', type: string, name: string, dimension: number } & NodeData
type ObjectDefine = { kind: 'object-define', name: string, vars: VarType[], code: Block } & NodeData
type Block = { kind: 'block', actions: (Action | Block | StateOrLoop)[] } & NodeData
type Action = { kind: 'action', operation: Keyword | ReturnExp | Call | VariableReassignment | VariableAssignment | Block } & NodeData
// type Action = FunctionLikeMacroCall | Keyword | FunctionLikeMacro | BasicMacro | ReturnExp | Call | VariableReassignment | VariableAssignment | Block
// type FunctionLikeMacroCall = { kind: 'preprocess-call', name: string, content: string[] } & NodeData
// type FunctionLikeMacro = { kind: 'macro-function', args: (string | BigInt)[], replace: string, with: string } & NodeData
// type BasicMacro = { kind: 'macro-basic', replace: string, with: string } & NodeData
type ReturnExp = { kind: 'return', value: Expression } & NodeData
type Call = { kind: 'call', call: SubnameCall, type: Type } & NodeData
type SubnameCall = { kind: 'tail-call', name: string, type: Type, part: CallPart | undefined, tail_call: SubnameCall | undefined } & NodeData
type CallPart = { kind: 'function-call', args: Expression[] } & NodeData
type Subname = { kind: 'property', name: string, type: Type } & NodeData
type Variable = (VariableAssignment | VariableReassignment) & NodeData
type VariableReassignment = { kind: 'variable-reassignment', name: Subname, value: Expression, op: string } & NodeData & { lifetime: number }
type VariableAssignment = { kind: 'variable-assignment', assign: VarType, expression: Expression, op: string, id: number, type: Type } & NodeData & { lifetime: number }
type Expression = { kind: 'expression', value: ExpressionPart } & NodeData
// type Cast = { kind: 'cast', to: string, dimension: number } & NodeData
type ExpressionPart = (_Array | _Object | Call | SubnameCall | _Number | _String) & NodeData
type _Array = { kind: 'array', items: Expression[], type: Type } & NodeData
type _Object = { kind: 'object', values: { [key: string]: Expression }, type: Type } & NodeData
type _Number = { kind: 'number', value: number, type: ({ type: 'normal', value: 'number' } & Type) } & NodeData
type _String = { kind: 'string', value: string, type: ({ type: 'normal', value: 'string' } & Type) } & NodeData
type FunctionHeader = { type: string, name: string, vars: VarType[], function_header: boolean }
type __Function = { kind: 'function', code: Block, header: FunctionHeader } & FunctionHeader
type _Function = __Function & NodeData
type StateOrLoop = (Condition | WhileLoop | ForLoop) & NodeData
type Condition = { kind: 'condition', code: (Block | Action), condition: Call } & NodeData
type WhileLoop = { kind: 'while-loop', code: (Block | Action), condition: Call } & NodeData
type ForLoop = { kind: 'for-loop', code: (Block | Action), start: Variable, condition: Call, cycle: (Call | VariableReassignment) } & NodeData
type Keyword = { kind: 'keyword', value: string } & NodeData

type LiterallyAnything = Keyword | SubnameCall | CallPart | ExpressionPart | IncludeCall | StructDef | VarType | ObjectDefine | Block | Action | ReturnExp | Call | Subname | VariableReassignment | VariableAssignment | Expression | _Array | _Object | _Number | _String | _Function | StateOrLoop | Condition | WhileLoop | ForLoop
type NodeData = {
	kind: string
	scope: number[]
	text: string
	id: number
	my_scope: number
	is_pure: boolean
}

type Type =
	{ type: 'normal', value: string } |
	{ type: 'array', value: Type } |
	{ type: 'function', value: FunctionHeader } |
	{ type: 'object', value: { [key: string]: Type } } |
	{ type: 'dynamic' } |
	{ type: 'void' }

type Declaration = { name: string, id: number, is_pure: boolean, function: boolean, type: Type }
type ReferenceScope = { declarations: Declaration[], references: number[] }

type TypeIndex = { [key: string]: Type }
// each type has a property dictionary
type PropsIndex = { [key: string]: { [key: string]: Property } }

type GlobalState = {
	global_scope: number
	global_variable_index: number
	global_id: number
	scopes: ReferenceScope[]
	types: TypeIndex,
	object_props: PropsIndex
}

// how in the hell am i gonna handle this???

// Basically, 'static' properties are properties that act like invisible fields,
// such as `.size` for structs (aka objects) or `.length` for arrays.

type Property = { kind: 'static', owner: string, type: Type, is_pure: boolean } |
{ kind: 'native', owner: string, 
	transform: (state: GlobalState, object: LiterallyAnything) => [GlobalState, LiterallyAnything] }