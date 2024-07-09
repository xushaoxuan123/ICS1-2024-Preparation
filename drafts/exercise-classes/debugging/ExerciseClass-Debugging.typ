#import "@preview/touying:0.4.2": *
#import "@local/my-template:0.1.1": init-cn
#import "@preview/pinit:0.1.4": *

#show: init-cn

#let s = themes.dewdrop.register(
  aspect-ratio: "16-9",
  footer: [ICS1-2024],
  navigation: none,
  primary: rgb("#ac0224"),
)
#let s = (s.methods.info)(
  self: s,
  title: [习题课：VSCode LSP & Debugging & C/C++ Code Style],
  subtitle: [ICS1-2024],
  author: [huanchengfly],
  date: datetime.today(),
  institution: [中国人民大学],
)
// #let s = (s.methods.enable-handout-mode)(self: s)
#let (init, slides, touying-outline, alert, speaker-note) = utils.methods(s)
#show: init

#show strong: alert

#show raw: it => {
  show regex("pin\d"): it => pin(eval(it.text.slice(3)))
  it
}

#let (slide, empty-slide, title-slide, new-section-slide, focus-slide) = utils.slides(s)
#show: slides

= C/C++ 开发环境配置

== 编译器

Windows / Linux

== IDE (Visual Studio Code)

Visual Studio Code 配置

= Debugging

== 如何使用 VSCode 的调试 GUI

GDB / LLDB

= Code Style

== Formatting

Format

== Linting

Clang-tidy

== Naming Convention

Naming Convention

== Modularity

Modularity
