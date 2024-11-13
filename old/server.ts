//@ts-check

import net from 'node:net'
import f from 'ansi-colors'
import * as scenes from './scenes.ts'

type EasyObject = { [key: string | number]: any }

const sessions: {
    [key: string]: scenes.Session
} = JSON.parse(await Deno.lstat('data.json').catch(_ => false) ? await Deno.readTextFile('data.json') : '{}')

const wait = _ => new Promise(r => setTimeout(r, _))

const server = net.createServer((socket) => {
    let id = (Math.floor(Math.random() * 8999999999) + 1000000000).toString()

    if (!(id in sessions)) {
        sessions[id] = {
            temp_data: {
                catted: 0,
                running: false,
                color: false
            },
            save_data: {
                in_game: false,
                history: []
            }
        }
    }

    const c = (func, text) => sessions[id].temp_data.color ? func(text) : text

    let prompt = () => `${c(f.magentaBright, id)} ${c(f.greenBright, '$')}`
    const send = (x) => socket.write(x)

    console.log('Connected: ' + id)
    console.log(socket.remoteAddress)

    const introduction = () => {
        if (sessions[id].temp_data.color) socket.write('\x1bc')
        send(`
${c(f.greenBright, '---')} Welcome to Astral - the interactive netcat game.
${c(f.greenBright, '---')} Your IP address is ${socket.remoteAddress}.

    - Type ${c(f.black.bgCyan, 'HELP')} for a guide.
    - Type ${c(f.black.bgCyan, 'QUIT')} to exit at any time.
    - Enable color with the COLOR command.
    - Leave your CAPS LOCK on, it's fun!
    - ${(id in sessions) ? c(f.greenBright, 'Your data is safe, no need to re-import it.') : 'I couldn\'t find your session, please import your data if you had some.'}`.trim() +

            `\n\n${prompt()} `)
    }

    introduction()

    socket.on('data', async (_data) => {
        if (sessions[id].temp_data.running) return

        sessions[id].temp_data.running = true
        const data = _data.toString().trim()

        let is_ended = false
        let matched = false
        const end = () => {
            Deno.writeTextFile('data.json', JSON.stringify(sessions, null, 4))
            is_ended = true
            socket.end()
        }
        const is = (cmd, fn) => { if (command.toLowerCase() === cmd.toLowerCase()) { matched = true; return fn() } return (async () => { })() }

        const command = data.split(' ')[0].toUpperCase().trim()
        const args = data.substring(command.length + 1).trim()

        const scene = (name) => {
            sessions[id].save_data.history.push(name)
            scenes.scenes[name](data, socket, sessions[id])
        }

        await is('help', async _ => {
            if (!sessions[id].save_data.in_game) {
                send('This is Astreal.\n')
                await wait(2000)
                send('An interactive game, based on communication through TCP only.\n')
                await wait(1000)
                send('The game will take place in Atrola, a modern-era country.\n')
                await wait(2000)
                send('Once you see an object, you can know more about it by ' + c(f.greenBright.bold, 'touch') + 'ing it.\n')
                await wait(500)
                send('For example: ' + c(f.greenBright.bold, 'touch table') + '\n')
                await wait(2000)
                send('You can move around using ' +
                    `up down north west south east`.split(' ')
                        .map(e => `${c(f.bgBlueBright.bold, e)} (${c(f.greenBright, e.charAt(0))})`)
                        .join(' ') + '.\n')
                await wait(1000)
                send(`You can look around using the ${c(f.redBright.bold, 'look')} command.\n` +
                    `You can also specify details such as ${c(f.redBright.bold, 'look <object>')} or ${c(f.redBright.bold, 'look <north/south/east/west>')}.\n`)
                await wait(2000)
                send(`Use the help command in different places when you're clueless on what to do.\n`)
                send(`Use the ${c(f.bgYellowBright.bold, 'begin')} command to start your journey!\n`)
            }
        })
        
        await is('color', () => {
            const Colors = f.blueBright('C') + f.cyanBright('o') + f.greenBright('l') + 
            f.magentaBright('o') + f.redBright('r') + f.yellowBright('s')

            if(sessions[id].temp_data.color) {
                send('Colors disabled!')
                sessions[id].temp_data.color = false
            } else {
                socket.write('\x1bc')
                introduction()
                send('\n')
                send(Colors + ' enabled!')
                sessions[id].temp_data.color = true
            }
        })

        await is('begin', async _ => {
            if (sessions[id].temp_data.color) socket.write('\x1bc')
            send('Welcome to Atrola!\n')
            await wait(500)
            scene('main_apartment')
        })

        if (!sessions[id].save_data.in_game) is('load', _ => {
            if (args === '') {
                send(`Usage: load <session id>\nDeletes current session's data.`)
                sessions[id].temp_data.running = false
                return
            }

            const _id = parseInt(args)
            if (isNaN(_id) || (args.length !== 10)) {
                send('Invalid session ID, try again.')
            } else {
                send(`Replaced your session ID with [${_id}]!\n` +
                    `Previous data will be deleted.`)
                delete sessions[id]
                id = _id.toString()
            }
        })

        is('quit', _ => {
            send('\n[ ] See you later!')
            socket.write('\n\n')
            end()
        })

        is('meow', _ => { // fun
            switch (sessions[id].temp_data.catted) {
                case 0:
                    send('Hello there! Didn\'t know cats could play this game.')
                    break
                case 1:
                    send('Meow to you too.')
                    break
                case 2:
                    send('Quite a noisy cat, huh?')
                    break
                case 3:
                    send('Well, to be honest, I didn\'t expect you to know how to do anything else. You\'re a cat.')
                    break
                case 4:
                default:
                    send('You should at least try to play the game, don\'t you think?')
            }

            sessions[id].temp_data.catted++
            if (sessions[id].temp_data.catted > 4) sessions[id].temp_data.catted = 4
        })

        is('clear', _ => {
            socket.write('\x1bc')
        })

        if (!is_ended) {
            if (!matched) {
                socket.write(`${c(f.redBright, '[Error]')} Not found: ${command}`)
            }

            socket.write(`\n${prompt()} `)
        }

        sessions[id].temp_data.running = false
    })

    socket.on('end', () => {
        console.log('Client disconnected.')
    })
})

const port = 1800
// const host = '136.243.255.124'
const host = '0.0.0.0'

server.listen(port, host, () => {
    console.log(`Game server running at tcp://${host}:${port}`)
})