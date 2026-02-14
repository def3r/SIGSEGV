fn main() {
    let mut plain_text = std::env::args().nth(1).expect("Expected plain text");
    plain_text = plain_text.to_lowercase();
    let key = 3;

    let cipher_text = caeser_cipher(plain_text.clone(), key);
    println!("Caeser Cipher with key=3: {}", cipher_text);
    println!(
        "Caeser Cipher decrypted : {}",
        caeser_cipher_d(cipher_text.clone(), key)
    );
}

fn caeser_cipher(plain_text: String, key: i8) -> String {
    let mut res = String::from("");
    let byte_a = 'a' as u8;
    for c in plain_text.bytes() {
        if c < byte_a {
            res.push(c as char);
            continue;
        }

        let cipher_char = (('a' as u8) + ((c - ('a' as u8) + key as u8) % 26)) as char;
        res.push(cipher_char);
    }
    res
}

fn caeser_cipher_d(plain_text: String, key: i8) -> String {
    let mut res = String::from("");
    let byte_a = 'a' as u8;
    for c in plain_text.bytes() {
        if c < byte_a {
            res.push(c as char);
            continue;
        }

        let cipher_char =
            (('a' as u8) + ((c + ('a' as u8) - ('A' as u8) + 1 - key as u8) % 26)) as char;
        res.push(cipher_char);
    }
    res
}
