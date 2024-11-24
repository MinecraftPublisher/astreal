//@ts-check

type Main = (IncludeCall | StructDef | ObjectDefine | _Function | FunctionLikeMacro | BasicMacro | FunctionLikeMacroCall | Keyword)[]
type IncludeCall = { kind: 'include', path: string } & NodeData
type StructDef = { kind: 'type-definition', name: string, fields: VarType } & NodeData
type VarType = { kind: 'variable-type', type: string, name: string, dimension: number } & NodeData
type ObjectDefine = { kind: 'object-define', name: string, vars: VarType[], code: Block } & NodeData
type Block = { kind: 'block', actions: (Action | Block | StateOrLoop)[] } & NodeData
type Action = { kind: 'action', operation: FunctionLikeMacroCall | Keyword | FunctionLikeMacro | BasicMacro | ReturnExp | Call | VariableReassignment | VariableAssignment | Block } & NodeData
// type Action = FunctionLikeMacroCall | Keyword | FunctionLikeMacro | BasicMacro | ReturnExp | Call | VariableReassignment | VariableAssignment | Block
type FunctionLikeMacroCall = { kind: 'preprocess-call', name: string, content: string[] } & NodeData
type FunctionLikeMacro = { kind: 'macro-function', args: (string | BigInt)[], replace: string, with: string } & NodeData
type BasicMacro = { kind: 'macro-basic', replace: string, with: string } & NodeData
type ReturnExp = { kind: 'return', value: string } & NodeData
type Call = { kind: 'call', name: Subname } & NodeData
type Subname = { kind: 'property', parts: string[] } & NodeData
type Variable = (VariableAssignment | VariableReassignment) & NodeData
type VariableReassignment = { kind: 'variable-reassignment', name: string, value: Expression, op: string } & NodeData & { lifetime: number }
type VariableAssignment = { kind: 'variable-assignment', name: string, value: Expression, op: string, id: number } & NodeData & { lifetime: number }
type Expression = { kind: 'expression', cast?: Cast, value: ExpressionPart } & NodeData
type Cast = { kind: 'cast', to: string, dimension: number } & NodeData
type ExpressionPart = (_Array | _Object | Call | Subname | _Number | _String) & NodeData
type _Array = { kind: 'array', value: Expression[] } & NodeData
type _Object = { kind: 'object', value: { [key: string]: Expression } } & NodeData
type _Number = { kind: 'number', value: number } & NodeData
type _String = { kind: 'string', value: string } & NodeData
type _Function = { kind: 'function', type: string, name: string, vars: VarType[], code: Block } & NodeData
type StateOrLoop = (Condition | WhileLoop | ForLoop) & NodeData
type Condition = { kind: 'condition', code: (Block | Action), condition: Call } & NodeData
type WhileLoop = { kind: 'while-loop', code: (Block | Action), condition: Call } & NodeData
type ForLoop = { kind: 'for-loop', code: (Block | Action), start: Variable, condition: Call, cycle: Call } & NodeData
type Keyword = { kind: 'keyword', value: string } & NodeData

type LiterallyAnything = Keyword | ExpressionPart | IncludeCall | StructDef | VarType | ObjectDefine | Block | Action | FunctionLikeMacroCall | FunctionLikeMacro | BasicMacro | ReturnExp | Call | Subname | VariableReassignment | VariableAssignment | Expression | Cast | _Array | _Object | _Number | _String | _Function | StateOrLoop | Condition | WhileLoop | ForLoop
type NodeData = { kind: string, scope: number[], text: string, id: number, my_scope: number }

// const $rng = () => Math.floor(Math.random() * 888888888888) + 111111111111

function Iterate<T>(obj: T, func: Function) {
    for (let key of Object.keys(obj as Object)) {
        if (obj[key]?.kind) {
            obj[key] = func(obj[key])
        }

        if (obj[key]?.map) {
            obj[key] = obj[key].map((e: any[]) => func(e))
        }
    }

    return obj as T
}

type ReferenceScope = { declarations: [string, number][], references: number[] }

let global_state: { global_scope: number, global_variable_index: number, global_id: number, scopes: ReferenceScope[] } = {
    global_scope: 0,
    global_variable_index: 0,
    global_id: 0,
    scopes: [
        {
            declarations: [
                ['println', -1],
                ['<', -1],
                ['++', -1]
            ],
            references: []
        }
    ]
}

function AddScope(object: LiterallyAnything, _scope: number[] = []) {
    let scope = _scope
    if (!object.kind) return Iterate(object, e => AddScope(e, scope))
    if (object.kind) {
        object.scope = scope
        object.id = global_state.global_id++
    }

    object.my_scope = scope[scope.length - 1]
    if (object.kind === 'block') {
        scope = [...scope, global_state.global_scope++]
        object.my_scope = global_state.global_scope
        object.my_scope = global_state.global_scope - 1

        global_state.scopes[global_state.global_scope] = {
            declarations: [],
            references: []
        }
    }

    object = Iterate(object, e => AddScope(e, scope))

    if (object.kind === 'variable-assignment') {
        object.id = ++global_state.global_variable_index

        for (let i = 0; i < scope.length; i++) {
            if (!global_state.scopes[scope[i]].declarations.find(e => e[0] === object.name)) {
                global_state.scopes[scope[i]].declarations.push([object.name, object.id])
            }
        }

        // SANITY TODO: check declarations on each scope to find duplicate names and throw an error
    } else if (object.kind === 'for-loop') {
        if (object.start.kind === 'variable-assignment') {
            object.start.my_scope = object.code.my_scope
        }
    } else if (object.kind === 'function') {
        object.vars.map(e => e.scope = object.code.scope)
    } else if (object.kind === 'property') {
        // find variable index based on keyword

        let resolved_id: number | null = null

        for (let i = scope.length - 1; i >= 0; i--) {
            let x = global_state.scopes[scope[i]].declarations.find(e => e[0] === object.parts[0])
            if (!x) continue
            resolved_id = x[1]
            break
        }

        if (!resolved_id) {
            throw `Invalid variable name ${object.parts.join('.')}!\nPossible Reasons:\n  1. Variable used before declaration\n  2. `
        }

        global_state.scopes[scope[scope.length - 1]].references = [...new Set([...global_state.scopes[scope[scope.length - 1]].references, resolved_id])]
    } else if (object.kind === 'variable-reassignment') {
        let resolved_id: number | null = null

        for (let i = scope.length - 1; i >= 0; i--) {
            let x = global_state.scopes[scope[i]].declarations.find(e => e[0] === object.name)
            if (!x) continue
            resolved_id = x[1]
            break
        }

        if (!resolved_id) {
            throw `Variable ${object.name} not found in reassignment!`
        }

        global_state.scopes[scope[scope.length - 1]].references = [...new Set([...global_state.scopes[scope[scope.length - 1]].references, resolved_id])]
    }

    return object
}

function AssignLifetime(_object: LiterallyAnything) {
    let object = _object
    if (!object.kind) return Iterate(object, AssignLifetime)

    if (object.kind === 'variable-assignment') {
        object.lifetime = -5

        for (let i = object.my_scope; i < global_state.scopes.length; i++) {
            if (!global_state.scopes[i].references.includes(object.id)) continue
            object.lifetime = i
            break
        }

        console.log(object.name, 'assigned at', object.my_scope, 'has lifetime', object.lifetime)
        if (object.lifetime === -5) console.warn('Warning: Unused variable {' + object.name + '}')
    } else if (object.kind === 'for-loop') {
        object.start.lifetime = -5

        for (let i = 0; i < global_state.scopes.length; i++) {
            if (!global_state.scopes[i].references.includes(object.start.id)) continue
            object.start.lifetime = i
            break
        }
    }

    return Iterate(object, AssignLifetime)
}

// function DetectUnused(object: LiterallyAnything, top_block = 0) {
//     if (!object.kind) return Iterate(object, e => DetectUnused(e, top_block))

//     // if (object.kind !== 'variable-assignment')
// }

// let macro_database: { [key: string]: BasicMacro | FunctionLikeMacro } = {}

// function DetectMacros(object: LiterallyAnything) {
//     if (object.kind === 'macro-basic' || object.kind === 'macro-function') macro_database[object.replace] = object

//     return Iterate(object, DetectMacros)
// }

// function FillMacros(object: LiterallyAnything, replace_db: { [key: string]: string } = {}) { // TODO: 
//     let output = object

//     if (object.kind === 'preprocess-call') {
//         let target_macro = macro_database[object.name]

//         if (!target_macro || target_macro.kind === 'macro-basic') return Iterate(object, e => FillMacros(object, replace_db))

//         if (target_macro.args.length !== object.content.length) {
//             if (target_macro.args.find(p => p === 1n)) {
//                 if (target_macro.args.length - 1 > object.content.length) {
//                     console.error(`Mismatched argument count for macro call {${object.text}}, expected ${target_macro.args.length - 1} plus an optional rest but got only ${object.content.length}.`)

//                     throw ''
//                 }

//                 let obj_build: { [key: string]: any } = {}
//                 obj_build.rest = object.content.slice(target_macro.args.length - 1)
//                 for (let i = 0; i < target_macro.args.length - 2; i++) {
//                     obj_build[target_macro.args[i] as string] = object.content[i]
//                 }

//                 return Iterate(parse(target_macro.with), e => FillMacros(e, { ...replace_db, ...obj_build })) as LiterallyAnything
//             } else {
//                 console.error(`Mismatched argument count for macro call {${object.text}}, expected ${target_macro.args.length} but got ${object.content.length}.`)

//                 throw ''
//             }
//         }

//         let obj_build: { [key: string]: any } = {}
//         for (let i = 0; i < target_macro.args.length - 1; i++) {
//             obj_build[target_macro.args[i] as string] = object.content[i]
//         }

//         // No rest args
//         return Iterate(parse(target_macro.with), e => FillMacros(e, { ...replace_db, ...obj_build })) as LiterallyAnything
//     }

//     for(let key of Object.keys(object)) {
//         if(key !== 'kind' && typeof(object[key]) === 'string') {
//             if(replace_db[object[key]]) object[key] = replace_db[key]
//         }
//     }

//     return Iterate(object, e => FillMacros(e, replace_db))
// }

import * as PARSER from './parser.js'
import color from 'json-colorizer'
const colorize = color.colorize

const parse = PARSER.parse

console.time('Parsing')

let result: Main = parse(`
    
$include(!stdio);

void main(int argc) {
    println("Hello world!");

    {
        println("Hello from inside this scope");

        var y = 4;
    }

    for(int i = 0; <(i, 10); ++(i)) {
        var x = i;

        {
            println(x);
        }

        x.delete();
    }
}

`).map(e => AddScope(e)).map(e => AssignLifetime(e)) as Main

// console.log(colorize(result))
// console.log(global_state)