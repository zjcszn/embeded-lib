流程：

#### STEP1 发送 REQB 命令： 05 00

```m
trans: 05 00 00
recv:  50 00 00 00 00 d1 03 86 0c 00 80 80
```

#### STEP2 发送 ATTRIB 命令（非标）：1D 00 00 00 00 00 08 01 08

```markdown
trans: 1D 00 00 00 00 00 08 01 08
recv:  08
```

#### STEP3 发送读取 UID 命令：00 36 00 00 08

```markdown
trans: 00 36 00 00 08
recv:  XX XX XX XX XX XX XX XX 90 00
```

