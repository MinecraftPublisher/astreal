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