mod arg_list;
mod class_info;
mod compiler;
mod import_info;
mod method_info;
mod my_string;
mod object_info;
mod py_arg;
mod py_type;
mod script;
mod version_info;
use compiler::*;
use std::fs::File;
use std::io::prelude::*;
use version_info::*;

fn main() {
    /* new a version_info object */
    println!("(pikascript) packages installed:");
    let mut version_info = VersionInfo::new();
    version_info = VersionInfo::analyze_file(version_info, String::from("requestment.txt"));
    println!();

    println!("(pikascript) pika compiler:");
    /* new a compiler, sellect to path */
    let mut compiler = Compiler::new(String::from(""), String::from("pikascript-api/"));
    /* analyze file begin with main.py */
    compiler = Compiler::analyze_file(compiler, String::from("main"), false);
    /* compile packages in requestment.txt */
    for package in &version_info.package_list {
        compiler = Compiler::analyze_file(compiler, String::from(package.0), false);
    }
    println!();

    /* write the infomatrion to compiler-info */
    let mut compiler_info_file =
        File::create(format!("{}compiler-info.txt", compiler.dist_path)).unwrap();
    let compiler_info = format!("{:?}", compiler);
    compiler_info_file.write(compiler_info.as_bytes()).unwrap();
    /* make the -api.c file for each python class */
    for (_, class_info) in compiler.class_list.iter() {
        let api_file_path = format!("{}{}-api.c", compiler.dist_path, class_info.this_class_name);
        let mut f = File::create(api_file_path).unwrap();
        f.write("/* ******************************** */\n".as_bytes())
            .unwrap();
        f.write("/* Warning! Don't modify this file! */\n".as_bytes())
            .unwrap();
        f.write("/* ******************************** */\n".as_bytes())
            .unwrap();
        f.write(class_info.include().as_bytes()).unwrap();
        f.write("#include <stdio.h>\n".as_bytes()).unwrap();
        f.write("#include <stdlib.h>\n".as_bytes()).unwrap();
        f.write("#include \"BaseObj.h\"\n".as_bytes()).unwrap();
        f.write("\n".as_bytes()).unwrap();
        f.write(class_info.method_api_fn().as_bytes()).unwrap();
        f.write(class_info.new_class_fn().as_bytes()).unwrap();
        f.write("\n".as_bytes()).unwrap();
        if !class_info.is_package {
            let name = String::from(class_info.this_class_name.to_string());
            f.write(format!("Arg *{}(PikaObj *self){{\n", &name).as_bytes())
                .unwrap();
            f.write(
                format!(
                    "    return arg_setMetaObj(\"\", \"{}\", New_{});\n",
                    &name, &name
                )
                .as_bytes(),
            )
            .unwrap();
            f.write("}\n".as_bytes()).unwrap();
        }
    }
    /* make the .h file for each python class */
    for (_, class_info) in compiler.class_list.iter() {
        let api_file_path = format!("{}{}.h", compiler.dist_path, class_info.this_class_name);
        let mut f = File::create(api_file_path).unwrap();
        f.write("/* ******************************** */\n".as_bytes())
            .unwrap();
        f.write("/* Warning! Don't modify this file! */\n".as_bytes())
            .unwrap();
        f.write("/* ******************************** */\n".as_bytes())
            .unwrap();
        f.write(format!("#ifndef __{}__H\n", class_info.this_class_name).as_bytes())
            .unwrap();
        f.write(format!("#define __{}__H\n", class_info.this_class_name).as_bytes())
            .unwrap();
        f.write("#include <stdio.h>\n".as_bytes()).unwrap();
        f.write("#include <stdlib.h>\n".as_bytes()).unwrap();
        f.write("#include \"PikaObj.h\"\n".as_bytes()).unwrap();
        f.write("\n".as_bytes()).unwrap();
        let new_class_fn_declear = format!("{};\n", class_info.new_class_fn_name());
        f.write(new_class_fn_declear.as_bytes()).unwrap();
        f.write("\n".as_bytes()).unwrap();
        f.write(class_info.method_impl_declear().as_bytes())
            .unwrap();
        f.write("\n".as_bytes()).unwrap();
        f.write("#endif\n".as_bytes()).unwrap();
    }
    /* make the pikascript.c */
    let api_file_path = format!("{}pikaScript.c", compiler.dist_path);
    let mut f = File::create(api_file_path).unwrap();
    /* add head */
    f.write("/* ******************************** */\n".as_bytes())
        .unwrap();
    f.write("/* Warning! Don't modify this file! */\n".as_bytes())
        .unwrap();
    f.write("/* ******************************** */\n".as_bytes())
        .unwrap();
    /* add include */
    f.write("#include \"PikaMain.h\"\n".as_bytes()).unwrap();
    f.write("#include <stdio.h>\n".as_bytes()).unwrap();
    f.write("#include <stdlib.h>\n".as_bytes()).unwrap();
    f.write("\n".as_bytes()).unwrap();
    /* get script from main.py */
    let pika_main = compiler
        .class_list
        .get_mut(&"PikaMain".to_string())
        .unwrap();
    /* make the pikascript.c */
    f.write(pika_main.script_fn(version_info).as_bytes())
        .unwrap();

    /* make the pikascript.h */
    let api_file_path = format!("{}pikaScript.h", compiler.dist_path);
    let mut f = File::create(api_file_path).unwrap();
    f.write("/* ******************************** */\n".as_bytes())
        .unwrap();
    f.write("/* Warning! Don't modify this file! */\n".as_bytes())
        .unwrap();
    f.write("/* ******************************** */\n".as_bytes())
        .unwrap();
    f.write(format!("#ifndef __{}__H\n", "pikaScript").as_bytes())
        .unwrap();
    f.write(format!("#define __{}__H\n", "pikaScript").as_bytes())
        .unwrap();
    f.write("#include <stdio.h>\n".as_bytes()).unwrap();
    f.write("#include <stdlib.h>\n".as_bytes()).unwrap();
    f.write("#include \"PikaObj.h\"\n".as_bytes()).unwrap();
    f.write("#include \"PikaMain.h\"\n".as_bytes()).unwrap();
    f.write("\n".as_bytes()).unwrap();
    f.write("PikaObj * pikaScriptInit(void);\n".as_bytes())
        .unwrap();
    f.write("\n".as_bytes()).unwrap();
    f.write("#endif\n".as_bytes()).unwrap();
}
