#![allow(unused_variables)]
type File = String;

use std::io;

fn open(f: &mut File) -> bool {
    true
}

fn close(f: &mut File) -> bool {
    true
}

#[allow(dead_code)]
fn read(f: &mut File, save_to: &mut Vec<u8>) -> ! {
    unimplemented!()
}

fn greet(name: String) {
    println!("Hello, {}", name);
}

fn main() {
    let mut f1 = File::from("f1.txt");
    open(&mut f1);
    //read(&mut f1, &mut vec![]);
    close(&mut f1);

    let name = "pascal";
    //let name = String::from("Pascal");
    greet(name.to_string());

    let s = String::from("hello world");
    let len = s.len();
    let slice = &s[4..len];
    let slice = &s[4..];
    let slice = &s[..];

    let word = first_word(&s);
    println!("the frist word is: {}", word);

    let a = [1, 2, 3, 4, 5];
    let slice = &a[1..3];

    let mut s = String::from("Hello ");
    s.push_str("rust");
    println!("push_str() -> {}", s);

    s.push('!');
    println!("push() -> {}", s);

    s.insert(5, ',');
    println!("insert() -> {}", s);

    s.insert_str(6, " I like");
    println!("insert_str() -> {}", s);

    let tup = (500, 6.4, 1);
    let (x, y, z) = tup;
    let five_hundred = tup.0;
    let six_point_four = tup.1;
    let one = tup.2;
    println!("tup.0 is {}, tup.1 is {}, tup.2 is {}", five_hundred, six_point_four, one);
    println!("The value of y is : {}", y);

    let s1 = String::from("Hello");
    let (s2, len) = calculate_length(s1);
    println!("s2 is {}, len is {}", s2, len);

    //struct
    let f1 = Files {
        name: String::from("f1.txt"),
        data: Vec::new(),
    };

    let f1_name = &f1.name;
    let f1_length = &f1.data.len();

    println!("{:?}", f1);
    println!("{} is {} bytes long", f1_name, f1_length);

    let black = Color(0, 0, 0);
    let origin = Point(0, 0, 0);

    let c1 = PokerSuit::Spades(5);
    let c2: PokerSuit = PokerSuit::Diamonds(13);

    let five = Some(5);
    let six = plus_one(five);
    let none = plus_one(None);

    let a = [1, 2, 3, 4, 5];
    println!("Please enter an array index.");
    let mut index = String::new();
    io::stdin()
        .read_line(&mut index)
        .expect("Failed to read line");

    let index: usize = index.trim()
                        .parse()
                        .expect("Index entered was not a number");
    let element = a[index];
    println!("The value of the element at  index {} is: {}", index, element);
}

fn first_word(s: &String) -> &str {
    &s[..1]
}

fn calculate_length(s: String) -> (String, usize) {
    let length = s.len();
    (s, length)
}

#[derive(Debug)]
struct Files {
    name: String,
    data: Vec<u8>,
}

struct Color(i32, i32, i32);
struct Point(i32, i32, i32);

#[derive(Debug)]
enum PokerSuit {
    Clubs(u8),
    Spades(u8),
    Diamonds(u8),
    Hearts(u8),
}

fn plus_one(x: Option<i32>) -> Option<i32> {
    match x {
        None => None,
        Some(i) => Some(i + 1),
    }
}