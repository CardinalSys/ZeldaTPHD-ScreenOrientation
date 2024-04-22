extern crate winapi;

use memory_rs::external::process::Process;
use winapi::um::psapi::GetModuleFileNameExW;
use winapi::um::winnt::HANDLE;
use std::ptr::null_mut;
use std::ffi::OsString;
use std::os::windows::ffi::OsStringExt;
use std::rc::Rc;
use std::{fs, thread};
use std::time::Duration;
use regex::Regex;

fn main() {
    println!("Waiting for the game to start");
    let cemu = loop {
        if let Ok(p) = Process::new("Cemu.exe") {
            break Rc::new(p);
        };

        thread::sleep(Duration::from_secs(5));
    };
    println!("Game hooked");

    let h_process = cemu.h_process as HANDLE;

    let mut buffer = vec![0u16; 260];
    let n_size = buffer.len() as u32;

    let result = unsafe {
        GetModuleFileNameExW(
            h_process,
            null_mut(),
            buffer.as_mut_ptr(),
            n_size,
        )
    };

    if result > 0 {
        let exe_path = OsString::from_wide(&buffer[..result as usize]).to_string_lossy().into_owned();
        let log_path = exe_path.replace("Cemu.exe", "log.txt"); 
        let base_address = get_base_address(&log_path);
        change_screen_orientation(base_address, cemu);
    } else {
        println!("Failed to get module file name.");
    }
}

fn get_base_address(path: &str) -> usize {
    let contents = fs::read_to_string(path)
        .expect("Should have been able to read the file");

    let pattern = Regex::new(r"base:\s*0x([0-9a-fA-F]+)").unwrap();
    
    if let Some(caps) = pattern.captures(&contents) {
        let base_address_str = caps.get(1).unwrap().as_str().trim_start_matches('0');
        if let Ok(base_address) = usize::from_str_radix(base_address_str, 16) {
            println!("Base address: 0x{:x}", base_address);
            return base_address;
        }
    }

    return 0;
}

fn change_screen_orientation(base_address: usize, process: Rc<Process>){
    let address = base_address + 0x1012667D;
    let current_orientation: u32 = process.read_value(address, true);
    let mut new_orientation = 0; 
    if current_orientation == 0{
        print!("Changing orientation to GameCube/WiiU mode");
        new_orientation = 1;
    }
    else if current_orientation == 1{
        print!("Changing orientation to Wii mode");
        new_orientation = 0;
    }

    process.write_value(address, new_orientation, true);
    
}

