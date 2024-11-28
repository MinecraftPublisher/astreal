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
type _Array = { kind: 'array', items: Expression[], type } & NodeData
type _Object = { kind: 'object', values: { [key: string]: Expression } } & NodeData
type _Number = { kind: 'number', value: number } & NodeData
type _String = { kind: 'string', value: string } & NodeData
type FunctionHeader = { type: string, name: string, vars: VarType[], function_header: boolean }
type _Function = { kind: 'function', code: Block } & FunctionHeader & NodeData
type StateOrLoop = (Condition | WhileLoop | ForLoop) & NodeData
type Condition = { kind: 'condition', code: (Block | Action), condition: Call } & NodeData
type WhileLoop = { kind: 'while-loop', code: (Block | Action), condition: Call } & NodeData
type ForLoop = { kind: 'for-loop', code: (Block | Action), start: Variable, condition: Call, cycle: (Call | VariableReassignment) } & NodeData
type Keyword = { kind: 'keyword', value: string } & NodeData

type Type =
    { type: 'normal', value: string } |
    { type: 'array', value: Type } |
    { type: 'function', value: FunctionHeader } |
    { type: 'object', value: { [key: string]: Type } } |
    { type: 'dynamic' } |
    { type: 'alias', ref: string } |
    { type: 'void' }

type LiterallyAnything = Keyword | CallPart | ExpressionPart | IncludeCall | StructDef | VarType | ObjectDefine | Block | Action | ReturnExp | Call | Subname | VariableReassignment | VariableAssignment | Expression | Cast | _Array | _Object | _Number | _String | _Function | StateOrLoop | Condition | WhileLoop | ForLoop
type NodeData = {
    kind: string
    scope: number[]
    text: string
    id: number
    my_scope: number
    is_pure: boolean
}

import * as PARSER from './parser.js'
import color from 'json-colorizer'
const colorize = color.colorize

const parse = PARSER.parse

// const $rng = () => Math.floor(Math.random() * 888888888888) + 111111111111

function normalize_text(text: string) {
    const lines = text.replace(/\t/g, '    ').split('\n')
    let default_indent = Infinity

    for (const l of lines.slice(1)) {
        const f = l.match(/^ */g)
        if (!f) continue
        if (f[0].length < default_indent) default_indent = f[0].length
    }

    return [lines[0], ...lines.slice(1).map(e => e.replace(new RegExp(`^ {${default_indent}}`, 'g'), ''))].join('\n')
}

function Iterate<T>(obj: T, func: (e: LiterallyAnything) => (LiterallyAnything | undefined)) {
    for (let key of Object.keys(obj as Object)) {
        if (obj[key]?.kind) {
            obj[key] = func(obj[key])
        }

        if (obj[key]?.map) {
            obj[key] = obj[key].map((e: LiterallyAnything) => func(e)).filter(e => !!e)
        }
    }

    return obj as T
}

type Declaration = { name: string, id: number, is_pure: boolean, function: boolean }
type ReferenceScope = { declarations: Declaration[], references: number[] }
type TypeDefinition = { name: string, fields: { type: string, field: string, dimension: number }[] }

//@ts-ignore
const sizes_raw = (new TextDecoder().decode((await new Deno.Command('./yippee_helper', {
    args: ['size']
}).output()).stdout)).split('\n').filter(e => e !== '').map(e => e.split(' '))
let sizes = {}

let global_state: {
    global_scope: number
    global_variable_index: number
    global_id: number
    scopes: ReferenceScope[]
    types: { [key: string]: Type }
} = {
    global_scope: 1,
    global_variable_index: 0,
    global_id: 0,
    scopes: [
        {
            declarations: [
                { name: 'cat', id: -4, function: true, is_pure: true },
                { name: 'println', id: -3, function: true, is_pure: false },
                { name: '<', id: -2, function: true, is_pure: true },
                { name: '+', id: -1, function: true, is_pure: true }
            ],
            references: []
        },
        {
            declarations: [],
            references: []
        }
    ],
    types: {
        byte: {
            type: 'normal',
            value: 'byte'
        },
        char: {
            type: 'normal',
            value: 'byte'
        },
        int: {
            type: 'normal',
            value: 'int'
        },
        long: {
            type: 'normal',
            value: 'long'
        },
        var: {
            type: 'dynamic'
        },
        string: {
            type: 'array',
            value: {
                type: 'normal',
                value: 'char'
            }
        },
        pointer: {
            type: 'normal',
            value: 'ptr'
        },
        void: {
            type: 'void'
        }
    }
}

for (let i = 0; i < sizes_raw[0].length; i++) sizes[sizes_raw[0][i]] = parseInt(sizes_raw[1][i])

function _type_size(_type: Type): number | null {
    if (_type.type === 'alias') return _type_size(global_state.types[_type.ref])
    if (_type.type === 'array') return _type_size(global_state.types.pointer)
    if (_type.type === 'function') return _type_size(global_state.types.pointer)
    if (_type.type === 'object') return Object.values(_type.value).map(e => _type_size(e)).reduce((a, b) => a === null || b === null ? null : a + b, 0)
    if (_type.type === 'void') return 0
    if (_type.type === 'normal') {
        if (!(_type.value in sizes)) error({} as LiterallyAnything, `Type ${_type.value} does not exist as a built-in!`)
        return sizes[_type.value]
    }
    if (_type.type === 'dynamic') return null

    return null
}

const init_state = JSON.parse(JSON.stringify(global_state))

function array_purity<T>(x: (T & NodeData)[]) {
    return x.filter(e => !e.is_pure).length === 0
}

/**
 * Processing steps:
 *  1. Adding scopes (includes variable fixes, sanity checks and also type declaration) DONE
 *  2. Assigning variable lifetimes DONE
 *  3. Resolving array types DONE
 *  4. Resolving dynamic types
 */

function format_fields(fields: (VarType | FunctionHeader)[]) {
    let obj: { [key: string]: Type } = {}
    for (let f of fields) {
        if ('function_header' in f) {
            obj[f.name] = {
                type: 'function',
                value: f as FunctionHeader
            }
        } else {
            if (global_state.types[f.type]) {
                if (f.dimension === 0) obj[f.name] = global_state.types[f.type]
                else {
                    let out = global_state.types[f.type]

                    for (let i = 0; i < f.dimension; i++) {
                        out = {
                            type: 'array',
                            value: out
                        }
                    }

                    obj[f.name] = out
                }
            } else {
                if (f.dimension === 0) {
                    obj[f.name] = {
                        type: 'normal',
                        value: f.type
                    }
                } else {
                    let out: Type = {
                        type: 'normal',
                        value: f.type
                    }

                    for (let i = 0; i < f.dimension; i++) {
                        out = {
                            type: 'array',
                            value: out
                        }
                    }

                    obj[f.name] = out
                }
            }
        }
    }

    return obj
}

function error(obj: LiterallyAnything, text: string) {
    console.error('\n\n' + text + '\n')
    console.error(normalize_text(obj.text))
    console.error('\n')
    throw ''
}

function add_scope(object: LiterallyAnything, _scope: number[] = []) {
    let scope = _scope
    if (!object.kind) return Iterate(object, e => add_scope(e, scope))
    if (object.kind) {
        object.scope = scope
        object.id = global_state.global_id++
        object.is_pure = true // assume purity by default.
    }

    object.my_scope = scope[scope.length - 1]
    if (object.kind === 'block') {
        if (object.actions.length === 0) return undefined

        object.my_scope = global_state.global_scope
        scope = [...scope, global_state.global_scope++]

        if (!global_state.scopes[global_state.global_scope]) global_state.scopes[global_state.global_scope] = {
            declarations: [],
            references: []
        }
    }

    if (object.kind === 'function') {
        global_state.scopes[0].declarations.push({
            name: object.name,
            id: object.id,
            function: true,
            is_pure: object.code.is_pure
        })

        // Include function arguments inside the function block scope.
        const block_scope = global_state.global_scope

        for(let a of object.vars) {
            global_state.scopes[global_state.global_scope].declarations.push({
                function: false,
                is_pure: true,
                id: global_state.global_variable_index++,
                name: a.name
            })
        }
    }

    object = Iterate(object, e => add_scope(e, scope))

    if (object.kind === 'action') {
        if (!object.operation) return undefined
        object.is_pure = object.operation.is_pure
    } else if (object.kind === 'variable-assignment') {
        object.id = ++global_state.global_variable_index

        const temp_scope = [0, ...scope]
        for (let i = 0; i < temp_scope.length; i++) {
            if (!global_state.scopes[temp_scope[i]].declarations.find(e => e[0] === object.name)) {
                // variable definitions are, by default, pure.
                global_state.scopes[temp_scope[i]].declarations.push({
                    name: object.name,
                    id: object.id,
                    function: false,
                    is_pure: object.value.is_pure
                })
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
        if (object.type === 'var') {
            throw `Function return type cannot be var!`
        }

        object.vars.map(e => e.scope = object.code.scope)

        object.is_pure = object.code.is_pure
        global_state.scopes[0].declarations = global_state.scopes[0].declarations.map(e => e.name === object.name && e.function ? { ...e, is_pure: object.code.is_pure } : e)
    } else if (object.kind === 'property') {
        // find variable index based on keyword

        let resolved_id: Declaration | null = null
        let resolved_index: number = -1

        const temp_scope = [0, ...scope]
        for (let i = temp_scope.length - 1; i >= 0; i--) {
            const x = global_state.scopes[temp_scope[i]].declarations.find(e => e.name === object.parts[0])
            if (!x) continue
            resolved_id = x
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

        object.is_pure = resolved_id.is_pure
        if (!resolved_id.function && resolved_index <= object.scope[object.scope.length - 1] && resolved_index === 0) object.is_pure = false

        global_state.scopes[scope[scope.length - 1]].references = [...new Set([...global_state.scopes[scope[scope.length - 1]].references, resolved_id.id])]
    } else if (object.kind === 'variable-reassignment') {
        let resolved_id: number | null = null
        let variable_scope = -1

        const temp_scope = [0, ...scope]
        for (let i = temp_scope.length - 1; i >= 0; i--) {
            const x = global_state.scopes[temp_scope[i]].declarations.find(e => e.name === object.name.parts[0] && !e.function)
            if (!x) continue
            resolved_id = x.id
            variable_scope = i
            break
        }

        if (!resolved_id) {
            throw `Variable ${object.name.parts.join('.')} not found in reassignment!`
        }

        if (variable_scope <= scope[scope.length - 1]) {
            // reassigning a function from a higher scope leads to impurity.
            object.is_pure = false
        }

        if (!object.value.is_pure) object.is_pure = false

        global_state.scopes[scope[scope.length - 1]].references = [...new Set([...global_state.scopes[scope[scope.length - 1]].references, resolved_id])]
    } else if (object.kind === 'call') {
        object.is_pure = object.name.is_pure && array_purity(object.calls)
    } else if (object.kind === 'include') {
        object.is_pure = false
    } else if (object.kind === 'expression') {
        object.is_pure = object.value.is_pure
    } else if (object.kind === 'function-call') {
        object.is_pure = array_purity(object.args)
    } else if (object.kind === 'block') {
        object.is_pure = array_purity(object.actions)
    } else if (object.kind === 'condition') {
        object.is_pure = object.condition.is_pure && object.code && object.code.is_pure
    } else if (object.kind === 'return') {
        object.is_pure = object.value.is_pure
    } else if (object.kind === 'while-loop') {
        object.is_pure = object.condition.is_pure && object.code.is_pure
    } else if (object.kind === 'array') {
        object.is_pure = array_purity(object.items)
    } else if (object.kind === 'object') {
        object.is_pure = array_purity(Object.values(object.values))
    } else if (object.kind === 'type-definition') {
        if (object.fields.length === 1 && object.fields[0].name === '_') {
            if (global_state.types[object.name]) {
                error(object, `Type ${object.name} already exists!`)
            }

            if (!global_state.types[object.fields[0].type]) {
                error(object, `Type ${object.fields[0].type} does not exist!`)
            }

            if (!('function_header' in object.fields[0])) global_state.types[object.name] = global_state.types[object.fields[0].type]
            else {
                global_state.types[object.name] = {
                    type: 'function',
                    value: object.fields[0]
                }
            }
        } else {
            const final: Type = {
                type: 'object',
                value: format_fields(object.fields)
            }

            if (global_state.types[object.name]) {
                error(object, `Type ${object.name} already exists!`)
            }

            global_state.types[object.name] = final
        }
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
    } else if (object.kind === 'block') {
        if (object.actions.length === 0) return undefined
    } else if (object.kind === 'condition') {
        if (!object.code) {
            if (!object.condition.is_pure) {
                error(object, 'Empty if statement contains impure code. Consider placing the code outside of the condition\'s clause.')
            }

            return undefined
        }
    }

    return Iterate(object, assign_lifetime)
}

function lifetime_repetition(object: LiterallyAnything) {
    had_unused = false
    let out = assign_lifetime(add_scope(object) as LiterallyAnything) as LiterallyAnything

    while (had_unused) {
        had_unused = false
        const prev_types = global_state.types
        global_state = JSON.parse(JSON.stringify(init_state))
        global_state.types = prev_types
        out = assign_lifetime(add_scope(out) as LiterallyAnything) as LiterallyAnything
    }

    return out
}

function resolve_types(object: LiterallyAnything) {
    // builtin types:
    // char (u1), bool (u1), short (i2), 
}

// const parse = (x) => ([] as LiterallyAnything[])

console.time('Parsing')

//@ts-ignore
const result: Main = parse(await Deno.readTextFile('input.yip')).map(e => lifetime_repetition(e)) as Main

// TODO: Make a colorizer using the same PEG grammar,
//       just alter it to output colored text instead of code.

function color_log(obj) {
    console.log(colorize(obj, { indent: 4 }))
}

color_log(result)
console.warn(global_state)

// TODO: One day I will become future me.