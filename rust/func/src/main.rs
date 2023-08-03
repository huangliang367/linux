#![allow(unused)]
enum Message {
    Quit,
    Move {x: i32, y: i32},
    Write(String),
    ChangeColor(i32, i32, i32),
}

impl Message {
    fn call(&self) {
        
    }
}

#[derive(Debug)]
pub struct Rectangle {
    width: u32,
    height: u32,
}

impl Rectangle {
    fn area(&self) -> u32 {
        self.width * self.height
    }

    fn can_hold(&self, other: &Rectangle) -> bool {
        self.width > other.width && self.height > other.height
    }

    pub fn new(width: u32, height: u32) -> Self {
        Rectangle { width, height }
    }

    pub fn width(&self) -> u32 {
        return self.width;
    }
}

fn main() {
    let rect1 = Rectangle {width: 30, height: 50};
    let rect2: Rectangle = Rectangle {width: 10, height: 40};
    let rect3 = Rectangle::new(35, 55);

    println!("The area of the rectangle is {} square pixels.",
        rect1.area());
    println!("Can rect1 hold rect2 ? {}", rect1.can_hold(&rect2));
    println!("{}", rect3.width());

    let m = Message::Write(String::from("hello"));
    m.call();
}
