use std::io;
fn main() {
    let a = [1, 2, 3, 4, 5];

    println!("Please enter an array index.");

    let mut index = String::new();
    io::stdin()
        .read_line(&mut index)
        .expect("Failed to read line");

    let index: usize = index
                        .trim()
                        .parse()
                        .expect("Index entered was not a number");
    let element = a[index];

    println!("The value of the element at index{} is: {}", index, element);

    let array: [String; 8] = std::array::from_fn(|i| String::from("rust is good!"));
    println!("{:?}", array);

    let one = [1, 2, 3];
    let two: [u8; 3] = [1, 2, 3];
    let blank1 = [0; 3];
    let blank2: [u8; 3] = [0; 3];

    let arrays: [[u8; 3]; 4] = [one, two, blank1, blank2];
    for a in &arrays {
        print!("{:?}: ", a);
        for n in a.iter() {
            print!("\t{} + 10 = {}", n, n + 10);
        }

        let mut sum = 0;
        for i in 0..a.len() {
            sum += a[i];
        }

        println!("\t({:?} = {})", a, sum);
    }

}
