//@ts-check

type Main = (IncludeCall | StructDef | ObjectDefine | _Function | Keyword)[]
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
type Call = { kind: 'call', name: Subname, calls: CallPart[] } & NodeData
type CallPart = { kind: 'function-call', args: Expression[] } & NodeData
type Subname = { kind: 'property', parts: string[] } & NodeData
type Variable = (VariableAssignment | VariableReassignment) & NodeData
type VariableReassignment = { kind: 'variable-reassignment', name: Subname, value: Expression, op: string } & NodeData & { lifetime: number }
type VariableAssignment = { kind: 'variable-assignment', name: string, value: Expression, op: string, id: number } & NodeData & { lifetime: number }
type Expression = { kind: 'expression', cast?: Cast, value: ExpressionPart } & NodeData
type Cast = { kind: 'cast', to: string, dimension: number } & NodeData
type ExpressionPart = (_Array | _Object | Call | Subname | _Number | _String) & NodeData
type _Array = { kind: 'array', value: Expression[] } & NodeData
type _Object = { kind: 'object', value: { [key: string]: Expression } } & NodeData
type _Number = { kind: 'number', value: number } & NodeData
type _String = { kind: 'string', value: string } & NodeData
type FunctionHeader = { type: string, name: string, vars: VarType[] }
type _Function = { kind: 'function', code: Block } & FunctionHeader & NodeData
type StateOrLoop = (Condition | WhileLoop | ForLoop) & NodeData
type Condition = { kind: 'condition', code: (Block | Action), condition: Call } & NodeData
type WhileLoop = { kind: 'while-loop', code: (Block | Action), condition: Call } & NodeData
type ForLoop = { kind: 'for-loop', code: (Block | Action), start: Variable, condition: Call, cycle: (Call | VariableReassignment) } & NodeData
type Keyword = { kind: 'keyword', value: string } & NodeData

type LiterallyAnything = Keyword | CallPart | ExpressionPart | IncludeCall | StructDef | VarType | ObjectDefine | Block | Action | ReturnExp | Call | Subname | VariableReassignment | VariableAssignment | Expression | Cast | _Array | _Object | _Number | _String | _Function | StateOrLoop | Condition | WhileLoop | ForLoop
type NodeData = {
    kind: string
    scope: number[]
    text: string
    id: number
    my_scope: number
    is_pure: boolean
}

// const $rng = () => Math.floor(Math.random() * 888888888888) + 111111111111

function Iterate<T>(obj: T, func: Function) {
    for (let key of Object.keys(obj as Object)) {
        if (obj[key]?.kind) {
            obj[key] = func(obj[key])
        }

        if (obj[key]?.map) {
            obj[key] = obj[key].map((e: any[]) => func(e)).filter(e => !!e)
        }
    }

    return obj as T
}

//                                     name    id      is_pure
type ReferenceScope = { declarations: [string, number, boolean][], references: number[] }
type TypeDefinition = { name: string, fields: { type: string, field: string, dimension: number }[] }

let global_state: {
    global_scope: number
    global_variable_index: number
    global_id: number
    scopes: ReferenceScope[]
    types: { [key: string]: TypeDefinition }
} = {
    global_scope: 0,
    global_variable_index: 0,
    global_id: 0,
    scopes: [
        {
            declarations: [
                ['println', -3, false],
                ['<', -2, true],
                ['+', -1, true]
            ],
            references: []
        }
    ],
    types: {}
}

let init_state = JSON.parse(JSON.stringify(global_state))

function array_purity(x: (any & NodeData)[]) {
    return x.filter(e => !e.is_pure).length === 0
}

/**
 * Processing steps:
 *  1. Adding scopes (includes variable fixes, sanity checks and also type declaration)
 *  2. Assigning variable lifetimes
 *  3. Resolving array types
 */

function add_scope(object: LiterallyAnything, parent: LiterallyAnything, _scope: number[] = []) {
    let scope = _scope
    if (!object.kind) return Iterate(object, e => add_scope(e, object, scope))
    if (object.kind) {
        object.scope = scope
        object.id = global_state.global_id++
    }

    object.my_scope = scope[scope.length - 1]
    if (object.kind === 'block') {
        if (object.actions.length === 0) return undefined

        scope = [...scope, global_state.global_scope++]
        object.my_scope = global_state.global_scope
        object.my_scope = global_state.global_scope - 1

        global_state.scopes[global_state.global_scope] = {
            declarations: [],
            references: []
        }
    }

    object = Iterate(object, e => add_scope(e, object, scope))
    object.is_pure = true // assume purity by default.

    if (object.kind === 'action') {
        if (!object.operation) return undefined
    }

    if (object.kind === 'variable-assignment') {
        object.id = ++global_state.global_variable_index

        for (let i = 0; i < scope.length; i++) {
            if (!global_state.scopes[scope[i]].declarations.find(e => e[0] === object.name)) {
                // variable definitions are, by default, pure.
                global_state.scopes[scope[i]].declarations.push([object.name, object.id, true])
            }
        }

        // SANITY TODO: check declarations on each scope to find duplicate names and throw an error
    } else if (object.kind === 'for-loop') {
        if (object.start.kind === 'variable-assignment') {
            // it was this easy?? it took me 2 days to fix the for-loop lifetime problem...
            // and it was all just a simple assignment?
            object.start.my_scope = object.code.my_scope
        }

        object.is_pure = object.start.is_pure && object.condition.is_pure &&
            object.cycle.is_pure && object.code.is_pure
    } else if (object.kind === 'function') {
        if (object.text === 'var') {
            throw `Function return type cannot be var!`
        }

        object.vars.map(e => e.scope = object.code.scope)

        object.is_pure = object.code.is_pure
        global_state.scopes[0].declarations.push([object.name, object.id, object.code.is_pure])
    } else if (object.kind === 'property') {
        // find variable index based on keyword

        let resolved_id: number | null = null
        let resolved_index: number = -1

        for (let i = scope.length - 1; i >= 0; i--) {
            let x = global_state.scopes[scope[i]].declarations.find(e => e[0] === object.parts[0])
            if (!x) continue
            resolved_id = x[1]
            resolved_index = i
            break
        }

        if (!resolved_id) {
            console.error(`\nInvalid variable name ${object.parts.join('.')}!
Possible Reasons:
  1. Variable used before declaration
  2. Illegal characters used in name\n`)
            console.error(global_state)
            throw ''
        }

        object.is_pure = global_state.scopes[resolved_index].declarations.find(e => e[1] === resolved_id)![2]

        global_state.scopes[scope[scope.length - 1]].references = [...new Set([...global_state.scopes[scope[scope.length - 1]].references, resolved_id])]
    } else if (object.kind === 'variable-reassignment') {
        let resolved_id: number | null = null
        let variable_scope = -1

        for (let i = scope.length - 1; i >= 0; i--) {
            let x = global_state.scopes[scope[i]].declarations.find(e => e[0] === object.name.parts[0])
            if (!x) continue
            resolved_id = x[1]
            variable_scope = i
            break
        }

        if (!resolved_id) {
            throw `Variable ${object.name} not found in reassignment!`
        }

        if (variable_scope <= scope[scope.length - 1]) {
            // reassigning a function from a higher scope leads to impurity.
            object.is_pure = false
        }

        global_state.scopes[scope[scope.length - 1]].references = [...new Set([...global_state.scopes[scope[scope.length - 1]].references, resolved_id])]
    } else if (object.kind === 'call') {
        object.is_pure = object.name.is_pure && array_purity(object.calls)
    } else if (object.kind === 'include') {
        object.is_pure = false
    } else if (object.kind === 'action') {
        object.is_pure = object.operation.is_pure
    } else if (object.kind === 'expression') {
        object.is_pure = object.value.is_pure
    } else if (object.kind === 'function-call') {
        object.is_pure = array_purity(object.args)
    } else if (object.kind === 'block') {
        object.is_pure = array_purity(object.actions)
    } else if (object.kind === 'condition') {
        object.is_pure = object.condition.is_pure && object.code.is_pure
    } else if (object.kind === 'return') {
        object.is_pure = object.value.is_pure
    } else if (object.kind === 'while-loop') {
        object.is_pure = object.condition.is_pure && object.code.is_pure
    } else if (object.kind === 'array') {
        object.is_pure = array_purity(object.value)
    } else if (object.kind === 'object') {
        object.is_pure = array_purity(Object.values(object.value))
    }

    return object
}

let had_unused = false

function assign_lifetime(object: LiterallyAnything) {
    if (!object.kind) return Iterate(object, assign_lifetime)

    if (object.kind === 'variable-assignment') {
        // Q: why  -5?
        // A: why not?
        object.lifetime = -5

        for (let i = object.my_scope; i < global_state.scopes.length; i++) {
            if (!global_state.scopes[i].references.includes(object.id)) continue
            object.lifetime = i
            break
        }

        if (object.lifetime === -5) {
            had_unused = true
            console.warn('Warning: Unused variable {' + object.name + '}')
            return undefined
        }
    } else if (object.kind === 'for-loop') {
        object.start.lifetime = -5

        for (let i = 0; i < global_state.scopes.length; i++) {
            if (!global_state.scopes[i].references.includes(object.start.id)) continue
            object.start.lifetime = i
            break
        }
    }

    return Iterate(object, assign_lifetime)
}

function clean_pure(object: LiterallyAnything) {
    if(!object.kind) return Iterate(object, clean_pure)

    // TODO: Implement this. You also need to check parental purity, good luck future me!
}

function lifetime_repetition(object: LiterallyAnything) {
    had_unused = false
    let out = assign_lifetime(add_scope(object, { kind: 'block', actions: [object as Block], id: -100, is_pure: false, my_scope: 0, text: '', scope: [] }) as LiterallyAnything) as LiterallyAnything

    while (had_unused) {
        had_unused = false
        global_state = JSON.parse(JSON.stringify(init_state))
        out = assign_lifetime(add_scope(out, { kind: 'block', actions: [out as Block], id: -100, is_pure: false, my_scope: 0, text: '', scope: [] }) as LiterallyAnything) as LiterallyAnything
    }

    return out
}

function resolve_types(object: LiterallyAnything) {
    // builtin types:
    // char (u1), bool (u1), short (i2), 
}

import * as PARSER from './parser.js'
import color from 'json-colorizer'
const colorize = color.colorize

const parse = PARSER.parse

// const parse = (x) => ([] as LiterallyAnything[])

console.time('Parsing')

let result: Main = parse(`

import stdio;

void main() {
    println("Hello world!");

    var x = 5;
    var y = +(34, x);

    {} {
        println(println(y));

        if(<(x, 2)) {
            var z = +(55, x);
        }
    } {} {} {} {}
}

`).map(e => lifetime_repetition(e)) as Main

// TODO: Make a colorizer using the same PEG grammar,
//       just alter it to output colored text instead of code.

console.log(colorize(result, { 'indent': 4 }))
console.warn(global_state)