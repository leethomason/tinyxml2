针对xml解释失败时输出格式精简, 新格式规则：
- 去掉 `Error=` 前缀，直接输出枚举名（`ErrorIDToName(error)` 的返回值）
- 去掉 `ErrorID=N (0xN)` 字段(无用内部字段)
- `(line N)` 仅当 `lineNum > 0` 时追加；`lineNum == 0` 时整段省略
- `<context>` 仅当调用方传入非空 format 时输出，前置 `: `
格式模板：`<ErrorName>[: <context>][ (line N)]`

如预期输出对照：
改前：
```
Error=XML_ERROR_PARSING ErrorID=15 (0xf) Line number=5
Error=XML_ERROR_MISMATCHED_ELEMENT ErrorID=14 (0xe) Line number=1: XMLElement name=child
Error=XML_ERROR_PARSING_ATTRIBUTE ErrorID=7 (0x7) Line number=7: XMLElement name=item
Error=XML_ERROR_PARSING_DECLARATION ErrorID=2 (0x2) Line number=3: XMLDeclaration value=xml
Error=XML_ERROR_PARSING_ELEMENT ErrorID=1 (0x1) Line number=9: XMLElement name=foo
Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename=foo.xml
Error=XML_ERROR_FILE_COULD_NOT_BE_OPENED ErrorID=4 (0x4) Line number=0: filename=<null>
Error=XML_ELEMENT_DEPTH_EXCEEDED ErrorID=18 (0x12) Line number=200: Element nesting is too deep.
```

改后：
```
XML_ERROR_PARSING (line 5)
XML_ERROR_MISMATCHED_ELEMENT: element <child> (line 1)
XML_ERROR_PARSING_ATTRIBUTE: in element <item> (line 7)
XML_ERROR_PARSING_DECLARATION: declaration <xml> (line 3)
XML_ERROR_PARSING_ELEMENT: element <foo> (line 9)
XML_ERROR_FILE_NOT_FOUND: "foo.xml"
XML_ERROR_FILE_COULD_NOT_BE_OPENED
XML_ELEMENT_DEPTH_EXCEEDED: element nesting is too deep (line 200)
```
- `xmltest.cpp` 中所有硬编码旧格式字符串的断言须同步更新为新格式,测试需覆盖本次修改内容

请帮我完成修改，过程中不用approval
