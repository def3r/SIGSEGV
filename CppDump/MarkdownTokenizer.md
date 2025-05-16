# Making Sense of it
```
this *is the *****ain't nothin*** this is cruel **\n
```

*Stack Trace:*
```
**
***
*****
*
```
*Tokens:* (len=8)
```
Token::TEXT             this
Token::ITALIC           *
Token::TEXT             is the
Token::NONE             *****
Token::TEXT             ain't nothin
Token::BOLD_ITALIC      ***
Token::TEXT              this is cruel
Token::BOLD             **
```

Accessing the stack from behind (`Dequeue`)
```
*       --- TOS             .index=1    .marker=*   .toErase=true
*****   --- TOS - 1         .index=3    .marker=*   .toErase=true
***
**
```
len(TOS) < len(TOS - 1)
        &&
TOS.at(0) == (TOS - 1).at(0) = "*"
    => len(TOS - 1) - len(TOS)
    => pop TOS
        => If TOS 
```
this <italic>is the <italic>****ain't nothin*** this is cruel **\n
```
*Tokens:* (len=8, correction=0, Insertion@indx(3), Deletion@indx(4))
```
Token::TEXT             this
Token::ITALIC           *
Token::TEXT             is the
Token::ITALIC           *
Token::TEXT             ain't nothin
Token::BOLD_ITALIC      ***
Token::TEXT              this is cruel
Token::BOLD             **
```
```
****   --- TOS
***    --- TOS - 1
**
```
len(TOS) > len(TOS - 1)
        &&
TOS.at(0) == (TOS - 1).at(0) = "*"
    => len(TOS) - len(TOS - 1)
    => pop TOS
```
this <italic>is the <italic>*<bold_italic>ain't nothin<bold_italic> this is cruel **\n
```
*Tokens:* (len=9, correction=1, Insertion@indx(4),~Deletion@indx(4)~)
```
Token::TEXT             this
Token::ITALIC           *
Token::TEXT             is the
Token::ITALIC           *
Token::BOLD_ITALIC      ***
Token::TEXT             ain't nothin
Token::BOLD_ITALIC      ***
Token::TEXT              this is cruel
Token::BOLD             **
```
```
*   --- TOS
**  --- TOS - 1
```
len(TOS) < len(TOS - 1)
        &&
TOS.at(0) == (TOS - 1).at(0) = "*"
    => len(TOS - 1) - len(TOS)
    => pop TOS
```
this <italic>is the <italic><italic><bold_italic>ain't nothin<bold_italic> this is cruel <italic>*\n
```
*Tokens:* (len=8, correction=3, Insertion@indx(4), Deletion@indx(10))
```
Token::TEXT             this
Token::ITALIC           *
Token::TEXT             is the
Token::ITALIC           *
Token::ITALIC           *
Token::BOLD_ITALIC      ***
Token::TEXT             ain't nothin
Token::BOLD_ITALIC      ***
Token::TEXT              this is cruel
Token::ITALIC           *
```
```
*  --- TOS
```
=> pop as TokenType::TEXT

==> pop and assign- TokenType::TEXT
```
Token::TEXT             this
Token::ITALIC           *
Token::TEXT             is the
Token::ITALIC           *
Token::ITALIC           *
Token::BOLD_ITALIC      ***
Token::TEXT             ain't nothin
Token::BOLD_ITALIC      ***
Token::TEXT              this is cruel
Token::ITALIC           *
Token::TEXT             *
```

> The Issue
`__this *should_ be** entirely bold__`
*res*
Token::BOLD             __
Token::TEXT             this
Token::ITALIC           *
Token::TEXT             should
Token::ITALIC           _
Token::TEXT              be
Token::BOLD             **
Token::TEXT              entirely bold
Token::TEXT             __

expected:
Token::BOLD             __
Token::TEXT             this
Token::ITALIC           *
Token::TEXT             should
Token::TEXT             _
Token::TEXT              be
Token::ITALIC           *
Token::TEXT             *
Token::TEXT              entirely bold
Token::BOLD             __

```
Stack:
__      --- TOS
*       --- TOS - 1
_
**
__
```

```
Stack:                          another stack?
_      --- TOS                  *
**     --- TOS - 1              __
__
```
check if TOS matches with any of `another stack?`

