import net from 'node:net'

export type Session = {
    temp_data: {
        catted: number
        running: boolean
        color: boolean
    }
    save_data: {
        in_game: boolean
        history: string[]
    }
}

export const scenes: { [key: string]: (data: string, socket: net.Socket, session: Session) => Session } = {
    apartment(data: string, socket: net.Socket, session: Session) {
        
        
        return session
    }
}