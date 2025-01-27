/// <reference path="types.d.ts"/>

export const types: TypeIndex = {
	byte: {
		type: 'normal',
		value: 'byte'
	},
	char: {
		type: 'normal',
		value: 'byte'
	},
	number: {
		type: 'normal',
		value: 'number'
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

// TODO: Tail call function type must reflect the return type, not the function itself.

function builtin(name: string, owner: string, args: VarType[]): Property {
	return {
		kind: 'static',
		owner,
		type: {
			type: 'function',
			value: {
				function_header: true,
				name,
				type: owner,
				vars: [
					{
						type: owner,
						name: 'me',
						dimension: 0,
					} as VarType,
					...args
				]
			}
		},
		is_pure: true
	}
}

export const props: PropsIndex = {
	'number': {
		'to_string': builtin('to_string', 'number', [])
	}
}