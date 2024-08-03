use std::io::prelude::*;
use std::io::{stdin, stdout};
use std::net::TcpStream;
use serde_json;
use std::result::Result;
use std::convert::TryFrom;
use std::str;
use termion;

enum ResponseState {
    WrongSecret,
    BadRequest,
    LoginFailed,
    Pairing,
    PairingSucceed,
    Full,
    GameOver,
    RivalLogout
}

impl TryFrom<i32> for ResponseState {
    type Error = ();

    fn try_from(value: i32) -> Result<Self, Self::Error> {
        match value {
            0 => Ok(ResponseState::WrongSecret),
            1 => Ok(ResponseState::BadRequest),
            2 => Ok(ResponseState::LoginFailed),
            3 => Ok(ResponseState::Pairing),
            4 => Ok(ResponseState::PairingSucceed),
            5 => Ok(ResponseState::Full),
            6 => Ok(ResponseState::GameOver),
            7 => Ok(ResponseState::RivalLogout),
            _ => Err(())
        }
    }
}


pub struct Client {
    secret: String,
    connection: Option<TcpStream>,
    response: serde_json::Value,
    running: bool,
    username: Option<String>
}

impl Client {
    pub fn new(secret: String) -> Client {
        Client {
            secret,
            connection: None,
            response: serde_json::Value::Null,
            running: false,
            username: None
        }
    }

    pub fn connect(&mut self, addr: &str) -> Result<(), std::io::Error> {
        self.connection = Some(TcpStream::connect(addr)?);
        self.running = true;
        Ok(())
    }

    pub fn login(&mut self, username: &str, password: &str) -> Result<(), std::io::Error> {
        let msg = serde_json::json!({
            "secret": self.secret,
            "type": 0,
            "name": username,
            "password": password
        });
        self.connection.as_mut().unwrap().write(msg.to_string().as_bytes())?;
        self.username = Some(String::from(username));
        Ok(())
    }

    pub fn receive(&mut self) -> bool {
        let mut data = [0; 128];
        let conn = self.connection.as_mut().unwrap();
        let n = conn.read(&mut data[..]).unwrap();
        if n == 0 {
            println!("server close connection");
            return false;
        }
        let response_str = str::from_utf8(&data[0..n]).unwrap();
        if let Ok(j) = serde_json::from_str(response_str) {
            self.response = j;
        } else {
            println!("parse json failed");
            return false;
        }
        true
    }

    pub fn handle_response(&mut self) -> Result<(), &'static str> {
        Self::clear_screen();
        self.print_title();
        let code = self.response["code"].as_u64().unwrap() as i32;
        let code_type = ResponseState::try_from(code).unwrap();
        match code_type {
            ResponseState::WrongSecret => Err("wrong secret"),
            ResponseState::BadRequest => Err("bad request"),
            ResponseState::LoginFailed => Err("login failed"),
            ResponseState::RivalLogout => Err("rival logout"),
            ResponseState::Full => Err("server busy"),
            ResponseState::Pairing => {
                println!("pairing");
                Ok(())
            },
            ResponseState::PairingSucceed => {
                match self.handle_gaming() {
                    Err(_) => Err("write socket error"),
                    Ok(()) => Ok(())
                }
            },
            ResponseState::GameOver => {
                self.handle_game_over();
                self.running = false;
                Ok(())
            }
        }
    }

    pub fn start_even_loop(&mut self) {
        assert!(self.connection.is_some());
        loop {
            if !self.receive() {
                self.running = false;
                break;
            }

            let stat = self.handle_response();
            match stat {
                Err(msg) => {
                    println!("{}", msg);
                    self.running = false;
                    break;
                }
                Ok(()) => {
                    if self.running == false {
                        break;
                    }
                }
            }
        }
    }

    pub fn print_title(&self) {
        print!("{}{}", termion::color::Bg(termion::color::Black),
               termion::color::Fg(termion::color::Magenta));
        println!("############################################################");
        println!("############################################################");
        println!("###################### W e l c o m e #######################");
        println!("### Dedicated to ZH.");
        println!("### Enter your pos in coordinate format `x,y` start from 1.");
        println!("### For example, ");
        println!("### > 1,1 will put your mark on the left-top of the chessboard.");
        if self.username.is_some() {
            println!("### User: {}", self.username.as_ref().unwrap());
        }
        println!("### \n");
        print!("{}{}", termion::color::Bg(termion::color::Reset),
               termion::color::Fg(termion::color::Reset));
    }

    fn handle_gaming(&mut self) -> Result<(), std::io::Error> {
        println!();
        let chessboard = self.response["chessboard"].as_str().unwrap();
        Client::print_chessboard(chessboard);
        println!();
        let current_acting = self.response["current_acting"].as_bool().unwrap();
        if !current_acting {
            println!("waiting for the other player");
            return Ok(());
        }
        let mut input = String::new();
        print!("> ");
        stdout().flush().expect("flush output stream failed");
        stdin().read_line(&mut input).expect("read user input failed");
        let mut coordinate = input.trim().split(",");
        let x = coordinate.next().unwrap().parse::<i32>().unwrap();
        let y = coordinate.next().unwrap().parse::<i32>().unwrap();
        let pos = (x - 1) * 3 + (y - 1);
        let request = serde_json::json!({
            "secret": self.secret,
            "type": 1,
            "pos": pos
        });
        let conn = self.connection.as_mut().unwrap();
        conn.write(request.to_string().as_bytes())?;
        Ok(())
    }

    fn handle_game_over(&self) {
        println!();
        let chessboard = self.response["chessboard"].as_str().unwrap();
        Client::print_chessboard(chessboard);
        println!("\ngame over");
        let tie = self.response["tie"].as_bool().unwrap();
        if tie {
            println!("tie");
        } else {
            let is_winner = self.response["is_winner"].as_bool().unwrap();
            if is_winner {
                println!("you win!");
            } else {
                println!("you lose!");
            }
        }
    }

    fn print_chessboard(board: &str) {
        let tmp = board.as_bytes();
        for i in 0..3 {
            for j in 0..3 {
                let c = match tmp[i*3+j] - '0' as u8 {
                    0 => '`',
                    1 => 'O',
                    2 => 'X',
                    _ => '`'
                };
                print!("{:3}", c)
            }
            println!();
        }
    }

    pub fn clear_screen() {
        print!("{}", termion::cursor::Goto(1,1));
        print!("{}", termion::clear::AfterCursor);
    }
}