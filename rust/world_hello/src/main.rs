use num::complex::Complex;

fn complex() {
    let a = Complex {re: 2.1, im: -1.2};
    let b = Complex::new(11.1, 22.2);
    let result = a + b;
    println!("{} + {}i", result.re, result.im);
}

fn dead_end() -> ! {
    panic!("panic");
}

fn main() {
    let penguin_data = "\
    common name, length (cm)
    Little penguin, 33
    Yellow-eyed penguin, 65
    Fiordland penguin, 60
    Invalid, data
    ";

    let records = penguin_data.lines();
    for (i, record) in records.enumerate() {
        if i == 0 || record.trim().len() == 0 {
            continue;
        }

        let fields: Vec<_> = record
            .split(',')
            .map(|field| field.trim())
            .collect();
        if cfg!(debug_assertions) {
            eprintln!("debug: {:?} -> {:?}",
                record, fields);
        }

        let name = fields[0];
        if let Ok(length) = fields[1].parse::<f32>() {
            println!("{}, {}cm", name, length);
        }
    }

    let a = 10;
    let b: i32 = 20;
    let mut c = 30i32;
    let d = 30_i32;
    let e = add(add(a, b), add(c, d));
    println!("(a + b) + (c + d) = {}", e);

    bit_opts();
 //   float_cmp();

    for i in 1..5 {
        println!("{}", i);
    }

    for i in 'a'..='z' {
        println!("{}", i);
    }

    complex();

    let x = '中';
    println!("size of x is {}", std::mem::size_of_val(&x));

    dead_end();
}

fn add(i: i32, j: i32) -> i32 {
    i + j
}

fn float_cmp() {
    let abc: (f32, f32, f32) = (0.1, 0.2, 0.3);
    let xyz: (f64, f64, f64) = (0.1, 0.2, 0.3);

    println!("abc (f32)");
    println!("  0.1 + 0.2: {:x}", (abc.0 + abc.1).to_bits());
    println!("        0.3: {:x}", (abc.2).to_bits());

    println!("xyz (f64)");
    println!("  0.1 + 0.2: {:x}", (xyz.0 + xyz.1).to_bits());
    println!("        0.3: {:x}", (xyz.2).to_bits());

    assert!(abc.0 + abc.1 == abc.2);
    assert!(xyz.0 + xyz.1 == xyz.2);
}

fn bit_opts() {
    let a: i32 = 2;
    let b: i32 = 3;

    println!("(a & b) value is {}", a & b);
    println!("(a | b) value is {}", a | b);
    println!("(a ^ b) value is {}", a ^ b);
    println!("(!b) value is {}", !b);
    println!("(a << b) value is {}", a << b);
    println!("(a >> b) value is {}", a >> b);

    let mut a = a;
    a <<= b;
    println!("(a << b) value is {}", a);
}