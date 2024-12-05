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
	int: {
		type: 'normal',
		value: 'int'
	},
	long: {
		type: 'normal',
		value: 'long'
	},
	number: {
		type: 'normal',
		value: 'float'
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

export const props: PropsIndex = {
	'number': {
		'balls': {
			kind: 'static',
			owner: 'number',
			type: {
				type: 'normal',
				value: 'bool'
			},
			is_pure: true
		}
	}
}