#import "@preview/touying:0.4.2": *
#import "@local/my-template:0.1.1": init-cn
#import "@preview/pinit:0.1.4": *

#show: init-cn

#let s = themes.dewdrop.register(
  aspect-ratio: "16-9",
  footer: [ICS1-2024],
  navigation: "mini-slides",
  primary: rgb("#ac0224"),
)
#let s = (s.methods.info)(
  self: s,
  title: [LinkLab 草案],
  subtitle: [ICS1-2024],
  author: [HuanChengFly],
  date: datetime.today(),
  institution: [中国人民大学],
)
// #let s = (s.methods.enable-handout-mode)(self: s)
#let (init, slides) = utils.methods(s)
#show: init
#show raw: it => {
  show regex("pin\d"): it => pin(eval(it.text.slice(3)))
  it
}

#let (slide, empty-slide) = utils.slides(s)
#show: slides

= 实验概述

== 实验目标

帮助同学们更好地理解程序链接的过程

==== Why?

#pause

- 链接很重要！
  - 对部署 C++ 项目、排查错误都有帮助
  - 作为一个独立的章节却没有 Lab……
- 之后的课程中不会再涉及到链接的内容
- 纯理论的讲解过于抽象
  - 大部分同学在学习后对链接的理解仍较为粗浅

== 实验内容

*实现一个简易的链接器*

主要是符号解析 & 重定位

只涉及静态链接 *动态链接过于复杂*

#pause

==== How?

ELF 文件？
- 过于复杂……

= 设计草案

== 草案1：a.out ❌

早期的 Unix 系统使用的可执行文件格式

优点：真实的格式！但……

*Linux 5.18 开始被正式移除*
- 无法直接运行 a.out 格式的可执行文件
- GCC 无法生成 a.out 格式的文件
- 无法使用现代工具链进行链接
- binutils 也不再支持 a.out 格式……

== 草案2：类 FLE ✔

Reference: #link("https://jyywiki.cn/OS/2024/lect19.md")[可执行文件和加载 by jyy\@NJU]

#block[
  #set text(size: 12pt)
  ```sh
  #!./exec pin1
  ```
  ```json
  {
      "type": ".exe",
      "symbols": {
          "_start": 15
      },
      ".load": [
          "🔢: ff ff ff ff ff ff ff",
          "🔢: ff ff ff ff ff ff ff",
          "🔢: 48 c7 c0 3c 00 00 00",
          "🔢: 48 c7 c7 2a 00 00 00",
          "             ^",
          "             |",
          "          This byte is return code (42).",
          "🔢: 0f 05 ff ff ff ff ff",
          "🔢: ff ff ff ff ff ff ff"
      ]
  }
  ```

  #pinit-point-from(1, offset-dy: 0pt, pin-dy: -3pt, body-dy: -4pt)[Shebang]
]

== 草案2：类 FLE ✔

主要组成部分：
/ 🔢: 代码
/ 📤: 符号
/ ❓: 重定位

自行实现：
- 编译器（调用 gcc 并将 gcc 输出的 .o 文件转换为 FLE 格式）
- 链接器（Lab 的内容，同学们自行实现，进行符号解析和重定位）
- 加载器（`mmap` and run!）

== 草案2：类 FLE ✔

优点：
- 人类友好

缺点：
- 过于“人类友好”……
  - 怎么解析和处理 JSON？（尤其是 C/C++）
- 本质上只是一个 toy
