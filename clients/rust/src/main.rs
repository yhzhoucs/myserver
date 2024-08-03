use std::thread::sleep;
use std::time::Duration;
use client::Client;
use std::io::{stdin, stdout, Write};
use termion::input::TermRead;

mod client;

fn read_userinfo() -> (String, String) {
    print!("username: ");
    stdout().flush().expect("flush output stream failed");
    let mut username = String::new();
    stdin().read_line(&mut username).expect("read input failed");
    print!("password: ");
    stdout().flush().expect("flush output stream failed");
    let mut stdin = stdin().lock();
    let mut stdout = stdout().lock();
    let password = stdin.read_passwd(&mut stdout).unwrap().unwrap();
    (username, password)
}

fn main() {
    let mut client = Client::new(String::from("KIA"));
    client.print_title();
    client.connect("127.0.0.1:8848").expect("connection failed");
    sleep(Duration::from_millis(200)); // wait for connection completion

    let (username, password) = read_userinfo();

    client.login(username.as_str().trim(), password.as_str()).expect("login failed");
    client.start_even_loop();
}
