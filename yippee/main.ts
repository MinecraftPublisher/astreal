type Main = (IncludeCall | StructDef | ObjectDefine | _Function | FunctionLikeMacro | BasicMacro | FunctionLikeMacroCall | BasicMacroCall)[]
type IncludeCall = { kind: 'include', path: string } & NodeData
type StructDef = { kind: 'type-definition', name: string, fields: VarType } & NodeData
type VarType = { kind: 'variable-type', type: string, name: string, dimension: number } & NodeData
type ObjectDefine = { kind: 'object-define', name: string, vars: VarType[], code: Block } & NodeData
type Block = { kind: 'block', actions: (Action | Block)[] } & NodeData
type Action = { kind: 'action', operation: FunctionLikeMacroCall | BasicMacroCall | FunctionLikeMacro | BasicMacro | ReturnExp | Call | VariableReassignment | VariableAssignment | Block } & NodeData
type FunctionLikeMacroCall = { kind: 'preprocess-call', name: string, content: string } & NodeData
type BasicMacroCall = { kind: 'preprocess-replace', name: string } & NodeData
type FunctionLikeMacro = { kind: 'macro-function', replace: string, with: string } & NodeData
type BasicMacro = { kind: 'macro-basic', replace: string, with: string } & NodeData
type ReturnExp = { kind: 'return', value: string } & NodeData
type Call = { kind: 'call', name: Subname } & NodeData
type Subname = { kind: 'property', parts: string[] } & NodeData
type VariableReassignment = { kind: 'variable-reassignment', name: string, value: Expression, op: string } & NodeData
type VariableAssignment = { kind: 'variable-assignment', name: string, value: Expression, op: string } & NodeData
type Expression = { kind: 'expression', cast?: Cast, value: ExpressionPart } & NodeData
type Cast = { kind: 'cast', to: string, dimension: number } & NodeData
type ExpressionPart = (_Array | _Object | Call | Subname | _Number | _String) & NodeData
type _Array = { kind: 'array', value: Expression[] } & NodeData
type _Object = { kind: 'object', value: { [key: string]: Expression } } & NodeData
type _Number = { kind: 'number', value: number } & NodeData
type _String = { kind: 'string', value: string } & NodeData
type _Function = { kind: 'function', type: string, name: string, vars: VarType[], code: Block } & NodeData

type NodeData = { kind: string, depth: number }

function AddDepth(object: NodeData, depth = 0) {
    if(object?.kind) object.depth = depth

    if(object?.kind === 'block') depth++

    for(let key of Object.keys(object)) {
        if(object[key]?.kind) {
            object[key] = AddDepth(object[key], depth)
        }

        if(object[key]?.map) {
            object[key] = object[key].map(e => AddDepth(object[key], depth))
        }
    }

    return object
}

import { parse } from './parser.js'

console.time('Parsing')

let result: Main = parse(`
$include(!stdio);

void main(int argc) {
    println("Hello world!");
}
`)

let newResult: Main = result.map(e => AddDepth(e)) as Main

console.log(JSON.stringify(newResult, null, 4))