/// <reference path="types.d.ts"/>

import * as PARSER from './parser.js'
import { color, colorize } from 'json-colorizer'
import *  as stdlib from './stdlib.ts'

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

function Iterate<T extends (Record<string, unknown> | unknown[])>(obj: T, func: (e: LiterallyAnything) => (LiterallyAnything | undefined)) {
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

//@ts-ignore
const sizes_raw = (new TextDecoder().decode((await new Deno.Command('./yippee_helper', {
	args: ['size']
}).output()).stdout)).split('\n').filter(e => e !== '').map(e => e.split(' '))
let sizes = {}

let __neg = -1

const vartype: (name: string, type: string) => VarType = (name, type) => { return { kind: 'variable-type', dimension: 0, is_pure: true, id: 0, type, name, text: `${type} ${name}`, my_scope: 0, scope: [0] } }
const func: (name: string, ...args: string[]) => Declaration = (name, ...args) => {
	return {
		name, id: __neg--, function: true, is_pure: true, type: {
			type: 'function', value: {
				name, type: 'string', vars: args.filter((e, i) => i % 2 == 0).map((e, i) => vartype(e, args[i * 2 + 1])), function_header: true
			}
		}
	}
}

let global_state: GlobalState = {
	global_scope: 0,
	global_variable_index: 0,
	global_id: 0,
	scopes: [
		{
			declarations: [
				func('cat', 'right', 'string', 'left', 'string'),
				func('println', 'text', 'string'),
				func('<', 'right', 'number', 'left', 'number'),
				func('+', 'right', 'number', 'left', 'number')
			],
			references: []
		},
		{
			declarations: [],
			references: []
		}
	],
	types: stdlib.types,
	object_props: stdlib.props
}

for (let i = 0; i < sizes_raw[0].length; i++) sizes[sizes_raw[0][i]] = parseInt(sizes_raw[1][i])

function _type_size(_type: Type): number | null {
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

function type_name(type: Type): string {
	if (type.type === 'array') return `_ARR_${type_name(type.value)}_ARR_`
	if (type.type === 'dynamic') return `_DYNAMIC_`
	if (type.type === 'function') return `_FUNC_${type.value.name}_FUNC_`
	if (type.type === 'normal') return `_NORMAL_${type.value}_NORMAL_`
	if (type.type === 'object') return `_OBJECT__${Object.keys(type.value).map(e => `$${e}_${type_name(type.value[e])}$`).join('_')}__OBJECT_`
	if (type.type === 'void') return `_UNKNOWN_`

	return '_UNKNOWN_'
}

function type_name_fmt(type: Type): string {
	if (type.type === 'array') return `${type_name_fmt(type.value)}[]`
	if (type.type === 'dynamic') return `var`
	if (type.type === 'function') return `${type.value.name}()`
	if (type.type === 'normal') return `${type.value}`
	if (type.type === 'object') return `{ ${Object.keys(type.value).map(e => `${e}: ${type_name_fmt(type.value[e])}`).join(', ')} }`
	// if (type.type === 'void') return `?`

	return '?'
}

const init_state = JSON.parse(JSON.stringify(global_state))

function array_purity<T>(x: (T & NodeData)[]) {
	return x.filter(e => !e.is_pure).length === 0
}

/*
 * Processing steps:
 *  1. Adding scopes (includes variable fixes, sanity checks and also type declaration) DONE
 *  2. Assigning variable lifetimes DONE
 *  3. Resolving array types DONE
 *  4. Resolving dynamic types
 */

function format_type(f: VarType | FunctionHeader): Type {
	if ('function_header' in f) {
		return {
			type: 'function',
			value: f as FunctionHeader
		}
	} else {
		if (global_state.types[f.type]) {
			if (f.dimension === 0) return global_state.types[f.type]
			else {
				let out = global_state.types[f.type]

				for (let i = 0; i < f.dimension; i++) {
					out = {
						type: 'array',
						value: out
					}
				}

				return out
			}
		} else {
			if (!global_state.types[f.type]) error(f, 'Unknown type!')

			if (f.dimension === 0) {
				return global_state.types[f.type]
			} else {
				let out: Type = global_state.types[f.type]

				for (let i = 0; i < f.dimension; i++) {
					out = {
						type: 'array',
						value: out
					}
				}

				return out
			}
		}
	}
}

function format_fields(fields: (VarType | FunctionHeader)[]) {
	let obj: { [key: string]: Type } = {}
	for (let f of fields) {
		obj[f.name] = format_type(f)
	}

	return obj
}

function error(obj: LiterallyAnything, text: string) {
	console.error('\n\n' + text + '\n')
	console.error(normalize_text(obj.text))
	console.error('\n')
	throw ''
}

function deep_equal(x, y) {
	const ok = Object.keys, tx = typeof x, ty = typeof y
	return x && y && tx === 'object' && tx === ty ? (
		ok(x).length === ok(y).length &&
		ok(x).every(key => deep_equal(x[key], y[key]))
	) : (x === y)
}

function invert_tail_call(_call: SubnameCall, scope) {
	let call = { ..._call }

	// find variable index based on keyword

	let resolved_id: Declaration | null = null
	let resolved_index: number = -1

	const temp_scope = [0, ...scope]
	for (let k = temp_scope.length - 1; k >= 0; k--) {
		const x = global_state.scopes[temp_scope[k]].declarations.find(e => e.name === call.name)
		if (!x) continue
		resolved_id = x
		resolved_index = k
		break
	}

	if (!resolved_id) {
		console.error(global_state)
		console.error(`\nInvalid variable name ${call.name}!
Possible Reasons:
1. Variable used before declaration
2. Illegal characters used in name\n`)
		throw ''
	}

	call.is_pure = resolved_id.is_pure
	// dammit i need to fix this. i need to resolve types aaaaa
	// object.type = global_state.types[resolved_id.]
	if (!resolved_id.function && resolved_index <= call.scope[call.scope.length - 1] && resolved_index === 0) call.is_pure = false
	if (resolved_id.function) call.is_pure = resolved_id.is_pure

	global_state.scopes[scope[scope.length - 1]].references = [...new Set([...global_state.scopes[scope[scope.length - 1]].references, resolved_id.id])]

	let flat: SubnameCall[] = []
	let prev_type = resolved_id.type

	call.type = resolved_id.type

	do {
		if (!call.type) {
			const found = global_state.object_props[type_name_fmt(prev_type)]
			if (found && found[call.name]) {
				const name_prop = found[call.name]
				
				if (name_prop) {
					if(name_prop.kind === 'static') {
						if(name_prop.type.type === 'function' && !call.part) {
							error(_call, `Expected function type, but got ${type_name_fmt(name_prop.type)}!`)
						}

						if(name_prop.type.type !== 'function' && call.part) {
							error(_call, `Got function for non-function type ${type_name_fmt(name_prop.type)}!`)
						}

						call.type = name_prop.type
						prev_type = name_prop.type
					}
				}
			} else if (prev_type.type === 'object' && prev_type.value[call.name]) {
				prev_type = prev_type.value[call.name]
				call.type = prev_type
			} else {
				error(_call, `Unknown property ${call.name} on type ${type_name_fmt(prev_type)}!`)
			}
		}

		if(call.part) if(call.part.args) call.part.args = add_scope(call.part.args, scope)
		const __call = call.tail_call
		delete call.tail_call
		call.scope = _call.scope
		call.id = global_state.global_id++
		call.my_scope = _call.my_scope
		// TODO: Purity analysis
		flat.push(call)
		call = __call as SubnameCall


	} while (call)

	flat.reverse()
	for (let i = 0; i < flat.length; i++) {
		if (!flat[i].part) { // has property
			const prev = flat[i + 1]
			if (prev) {
				prev.name = `${prev.name}.${flat[i].name}`
				// prev.tail_call = flat[i]
				prev.type = flat[i].type
				flat.splice(i, 1)
			}
		}
	}

	flat.reverse()
	for (let i = 0; i < flat.length; i++) {
		const p = flat[i].part
		if (p) {
			const prev = flat[i - 1]
			if (prev) {
				p.args.push({
					kind: 'expression',
					id: global_state.global_id++,
					my_scope: prev.my_scope,
					scope: prev.scope,
					is_pure: prev.is_pure,
					text: prev.text,
					value: prev
				})

				flat[i].part = p
				flat[i].name = `${type_name(prev.type)}:${flat[i].name}`

				flat.splice(i - 1, 1)

			}
		}
	}

	// TODO: Tail call type should be the type of the last property, not the first.

	return flat[0]
}

function add_scope(object: LiterallyAnything, _scope: number[] = []) {
	let scope = _scope
	if (!object.kind) return Iterate(object, e => add_scope(e, scope))

	object.scope = scope
	object.id = global_state.global_id++
	object.is_pure = true // assume purity by default.

	object.my_scope = scope[scope.length - 1]
	if (object.kind === 'block') {
		// if (object.actions.length === 0) return undefined

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
			is_pure: object.code.is_pure,
			type: {
				type: 'function',
				value: object.header
			}
		})

		// Include function arguments inside the function block scope.
		const block_scope = global_state.global_scope

		for (let a of object.vars) {
			if (!(a.type in global_state.types)) error(object, 'Function input type was not found!')
			if (a.type === 'var') error(object, 'Function input type cannot be var!')

			global_state.scopes[global_state.global_scope].declarations.push({
				function: false,
				is_pure: true,
				id: global_state.global_variable_index++,
				name: a.name,
				type: global_state.types[a.type]
			})
		}
	}

	if (object.kind === 'tail-call') {

	} else {
		object = Iterate(object, e => add_scope(e, scope))
	}

	if (object.kind === 'action') {
		if (!object.operation) return undefined
		object.my_scope = object.operation.my_scope
		object.is_pure = object.operation.is_pure
	} else if (object.kind === 'variable-assignment') {
		object.id = ++global_state.global_variable_index

		object.type = format_type(object.assign)

		if (object.assign.type === 'var' && object.assign.dimension !== 0) {
			error(object, 'Automatic typing cannot accept var arrays of any dimension')
		}

		// TODO: Assign types to property
		if (object.expression.value.type.type === 'void') {
			if (object.assign.type === 'var') {
				error(object, 'Dynamic variable type could not be detected from expression!')
			} else {
				object.expression.value.type = object.type
			}
		} else if (object.assign.type === 'var') {
			object.type = object.expression.value.type
		} else if (!deep_equal(object.expression.value.type, object.type)) {
			// Order of execution matters!
			error(object, 'Mismatched expression type in variable declaration')
		}

		const temp_scope = [0, ...scope]
		for (let i = 0; i < temp_scope.length; i++) {
			if (!global_state.scopes[temp_scope[i]].declarations.find(e => e[0] === object.assign.name)) {
				// variable definitions are, by default, pure.
				global_state.scopes[temp_scope[i]].declarations.push({
					name: object.assign.name,
					id: object.id,
					function: false,
					is_pure: object.expression.is_pure,
					type: object.assign.type === 'var' ? object.expression.value.type : global_state.types[object.assign.type]
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
		for (let k = temp_scope.length - 1; k >= 0; k--) {
			const x = global_state.scopes[temp_scope[k]].declarations.find(e => e.name === object.name)
			if (!x) continue
			resolved_id = x
			resolved_index = k
			break
		}

		if (!resolved_id) {
			error(object, `\nInvalid variable name ${object.name}!
Possible Reasons:
  1. Variable used before declaration
  2. Illegal characters used in name\n`)
			throw ''
		}

		object.is_pure = resolved_id.is_pure
		// dammit i need to fix this. i need to resolve types aaaaa
		// object.type = global_state.types[resolved_id.]
		if (!resolved_id.function && resolved_index <= object.scope[object.scope.length - 1] && resolved_index === 0) object.is_pure = false
		if (resolved_id.function) object.is_pure = resolved_id.is_pure

		global_state.scopes[scope[scope.length - 1]].references = [...new Set([...global_state.scopes[scope[scope.length - 1]].references, resolved_id.id])]
	} else if (object.kind === 'variable-reassignment') {
		let resolved_id: Declaration | null = null
		let variable_scope = -1

		const temp_scope = [0, ...scope]
		for (let i = temp_scope.length - 1; i >= 0; i--) {
			const x = global_state.scopes[temp_scope[i]].declarations.find(e => e.name === object.name.name && !e.function)
			if (!x) continue
			resolved_id = x
			variable_scope = i
			break
		}

		if (!resolved_id) {
			throw `Variable ${object.name.name} not found in reassignment!`
		}

		if (variable_scope <= scope[scope.length - 1]) {
			// reassigning a function from a higher scope leads to impurity.
			object.is_pure = false
		}

		if (!object.value.is_pure) object.is_pure = false
		if (resolved_id.type !== object.value.value.type) {
			error(object, 'New variable value does not match the initial value!')
		}

		global_state.scopes[scope[scope.length - 1]].references = [...new Set([...global_state.scopes[scope[scope.length - 1]].references, resolved_id.id])]
	} else if (object.kind === 'call') {
		object.is_pure = object.call.is_pure
		object.type = object.call.type

		object.call = invert_tail_call(object.call, _scope)
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
		if (object.items.length === 0) object.type = { type: 'void' }
		else object.type = { type: 'array', value: object.items[0].value.type }
		object.is_pure = array_purity(object.items)
	} else if (object.kind === 'object') {
		// i REALLY don't feel like even attempting to automatically detect struct types.
		// deal with it yourself.
		object.type = { type: 'void' }
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
	} else if (object.kind === 'number') {
		object.type = { type: 'normal', value: 'number' }
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
			console.warn('Warning: Unused variable {' + object.assign.name + '}')
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
	} else if (object.kind === 'function') {
		if (object.code.actions.length === 0) {
			object.code.actions.push({
				kind: "action",
				operation: {
					kind: 'call',
					call: {
						kind: 'tail-call',
						name: 'dummy',
						part: {
							scope: [],
							my_scope: -Infinity,
							id: -Infinity,
							is_pure: true,
							kind: 'function-call',
							args: [],
							text: '()'
						},
						scope: [],
						my_scope: -Infinity,
						id: -Infinity,
						is_pure: true,
						type: { type: 'void' },
						tail_call: undefined,
						text: 'dummy()'
					},
					text: 'dummy()',
					type: { type: 'void' },
					scope: [],
					my_scope: -Infinity,
					id: -Infinity,
					is_pure: true
				},
				text: "dummy()",
				scope: [],
				id: -Infinity,
				is_pure: true,
				my_scope: 0
			})
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