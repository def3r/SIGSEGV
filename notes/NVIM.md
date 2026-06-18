**Generic Info**<br>
https://app.element.io/#/room/#neovim-dev:matrix.org

Some useful info {{{

- GLOBALS: `src/nvim/globals.h`
  - curwin
  - firstbuf, lastbuf, curbuf -> linked list
  - EXTERN int State INIT( = MODE_NORMAL);
  - EXTERN int msg_scroll INIT( = false);        // msg_start() will scroll

- curbuf not of type Buffer(typdef of int) but buf_T (typedef of struct file_buffer)
- prefix b_ -> buffer property
- suffix _cb -> callback(?)
- pum/PUM -> pop up menu
- insert completion -> insexpand.c
- omni completion see `:h omnicompletion`
- `p` in `plines` stand for physical lines! (of on the screen lines)

buffer_defs.h:
- window_S:
  - w_curswant: Col  we want to be at(eg for up down motions)
// "w_topline", "w_leftcol" and "w_skipcol" specify the offsets for
// displaying the buffer.
  - w_topline: Buffer line num at top of the window
// curs_columns() -> syncing of w_cursor and the crusor on screen
// w_cursor-> logical cursor; (w_wroc, w_wcol)-> visual cursor
  - w_cursor: The byte cursor is on in data(idx of data)
  - w_wrow, w_wcol: the blinking pos of cursor on screen

data member:
  - lnum \[cursor] (int32_t): line number
  - b_no_eol_lnum (int32_t): ???

Callbacks:

| Callback | Function Def |
|-----------------|-------------|
| .movecursor | `src/nvim/terminal.c:term_movecursor` |
|.resize | `src/nvim/vterm/screen.c:resize_buffer` |


  - `updatecursor` calls movecursor callback

! handled:
  - void do_bang(int addr_count, exarg_T *eap, bool forceit, bool do_in, bool do_out)

}}}

[ISSUE #36027](https://github.com/neovim/neovim/issues/36027) `terminal`

[ISSUE #33480](https://github.com/neovim/neovim/issues/33480) `ansi color codes`

[ISSUE #5431 Terminal Improvements](https://github.com/neovim/neovim/issues/5431)
- [PR: Terminal improvements abandoned](https://github.com/neovim/neovim/pull/2683)

[ISSUE #12374](https://github.com/neovim/neovim/issues/12374) `Prevent cursor from moving after using an operator`
- [PR: unmerged, closed, insightful discussion](https://github.com/neovim/neovim/pull/21620)
- [YankAssasin: Plugin that does something similar](https://github.com/svban/YankAssassin.vim/blob/main/plugin/YankAssassin.vim)

[ISSUE #26861](https://github.com/neovim/neovim/issues/26861) `jobs/channel stats`

[ISSUE #18833](https://github.com/neovim/neovim/issues/18833) `jobstart fails if '\0' char`

[ISSUE #12428](https://github.com/neovim/neovim/issues/12428) `cmdline pum open while typing`

[ISSUE #19393](https://github.com/neovim/neovim/issues/19393) `RegGet and RegSet aucmd`

[ISSUE #1982](https://github.com/neovim/neovim/issues/1982) `undo collapse`

[ISSUE #20407](https://github.com/neovim/neovim/issues/20407) `gradient bg colors!`
- Possible, modern term emulators support 24-bit colors
- `ctermbg`, `guibg` -- but for terminals with 24 bit support: `termguicolors`
- `lerp` -> Linear Interpolation
- Blessing in disguise: [#37668](https://github.com/neovim/neovim/pull/37668)
- Issues:
  - Horizontal dir gradient is super expensive for normal buf bg
    1. Current impl has memset to display empty lines / space after EOL.
       Using horizontal grad actually needs each cell to be updated with
       it's specific color. i.e each cell needs to be drawn w/o memset.
       This probelm may be easily avoided in veryical dir gradient
  - If we only consider gradient for Statusline, CursorLine, Tabline,
    then the field `guibgfrom`, `guibgvia`, `guibgto` must be only valid for
    those and no other `hl`. This is useless for others and struct needs
    extra field just for 3 `hl`
- Proposal:
  - API:
    - most verbose: `hi StatusLine guibg='linear, #090909 20%, #171717 50%, #FFFFFF 80%` [On hold, v1 only supports linear dir]
    - `hi StatusLine guibg='#090909, #171717, #FFFFFF'`
    - `hi StatusLine guibg='#090909, #FFFFFF'`
    - `hi StatusLine guibg='#090909, #171717 30%, #FFFFFF'`
    - `hi StatusLine guibg='#090909'`
  - Illegal Stuff:
    - `hi StatusLine guibg='#090909 30%'` -> From but to where? Fallback to no grad
    - `hi StatusLine guibg='#090909 130%, #171717'` -> ???
    - `hi StatusLine guibg='gibberish, #090909 30%, #171717'` -> Fallback to linear
  - Impl:
    - To the `set_gui_color`, `''` are trimmed
    - Use an Array for no. of stops.
    ```c
    struct hl_grad {
      char dir; // [l]inear
      vec bgStops stops;
    };

    struct bgStops {
      color;
      fraction;
    }
    ```

[ISSUE #18756](https://github.com/neovim/neovim/issues/18756) `highlight over search results`

```lua 
  vim.api.nvim_create_autocmd('TermOpen', {
    group = nvim_terminal_augroup,
    desc = 'Delete the "[Process exited]" virtual text from buffer',
    callback = function(ev)
      local chan = vim.bo[ev.buf].channel
      local info = vim.api.nvim_get_chan_info(chan)
      if info.exitcode == nil or info.exitcode == -1 then
        print('open ' .. vim.inspect(vim.api.nvim_get_chan_info(chan)))
        print('TermOpen')
        vim.api.nvim_buf_clear_namespace(ev.buf, nvim_terminal_exitmsg_ns, 0, -1)
      end
    end,
  })
```

**Completed Issues:**<br>
{{{

[ISSUE #35331](https://github.com/neovim/neovim/issues/35331) `terminal draw offset related`
{{{
- [PR -> #37159](https://github.com/neovim/neovim/pull/37159)
- --clean, do :term<CR> btop, resize the window
- Interestingly, :term btop<CR> does not behave the same
- More interestingly, `btop` and `lazygit` have different offset!
- `.movecursor = term_movecursor` (terminal.c#212)
- Issue persists even after ignoring: -> refresh_size independent
```c
static void refresh_terminal(Terminal *term) // terminal.c
{
  ...
  refresh_size(term, buf);
  refresh_scrollback(term, buf);
  refresh_screen(term, buf);
  ...
}
```

- We meet again:
```c 
Integer nvim_open_term(Buffer buffer, Dict(open_term) *opts, Error *err)
  FUNC_API_SINCE(7)
  FUNC_API_TEXTLOCK_ALLOW_CMDWIN
{ ... }
```

- There are 2 resizes for term... I don't yet know why
```c 
static void term_resize(uint16_t width, uint16_t height, void *data) // channel.c
{
  Channel *chan = data;
  pty_proc_resize(&chan->stream.pty, width, height);
}

// AND
static void refresh_size(Terminal *term, buf_T *buf) // terminal.c
{
  if (!term->pending.resize || term->closed) {
    return;
  }
  term->pending.resize = false;
  int width, height;
  vterm_get_size(term->vt, &height, &width);
  term->invalid_start = 0;
  term->invalid_end = height;
  term->opts.resize_cb((uint16_t)width, (uint16_t)height, term->opts.data);
}
```

- A `maybe` suspect:
```c 
int pty_proc_spawn(PtyProc *ptyproc) // nvim/os/pty_proc_unix.c
  FUNC_ATTR_NONNULL_ALL
{ ... }
```
- UPDATE: Well, seems like even `:term lazygit` faces the same issue, when brought in `terminal (insert)` mode.
The resize is fine in `normal/visual block/viual/visual line`
- UPDATE: When in `normal mode`, fault resize happens only when the cur is at end of line
- `src/nvim/drawscreen.c` opens entirely new dimension of plausible errors
- `topframe`
// The window layout is kept in a tree of frames.  topframe points to the top
// of the tree.
EXTERN frame_T *topframe;      // top of the window frame tree
- SUS: `src/nvim/drawscreen.c:340` -> uncommenting doesnt resize, can be helpful
- SUS: `src/nvim/window.c:6772`
- SUS: `src/nvim/drawline.c:3249`
- BREAKING: in `drawline.c` adding `wp->w_leftcol = 0;` at line 1480 fixes the problem
  - 2216 of `drawscreen.c`: Anywhere inside win_update works;
  - 679 or anywhere inside da loop of `drawscreen.c`
  - if w_leftcol is +ve, window apears to be shifted. (Why?, time to solve this issue now)
  - 697 of `drawscreen.c`: Perfect pos
- ISSUE: in term mode having multiple lines and then resize while cursor is on last line of input of terminal shrinks the shell
- SOL: `src/nvim/move.c`:873
```c
  } else if (may_scroll
             && !wp->w_cline_folded
             && !(State & MODE_TERMINAL)) {
```
- LOG: `ILOG("SHIT: w_wcol=%d, extra=%d, width1=%d, new_leftcol=%d", (int)wp->w_wcol, (int)extra, width1, new_leftcol);`
- SOL: `src/nvim/screen.c`:718
```c
    statefields->pos = (VTermPos){0, 0}; // This is the pos for the terminal to draw from
```
- THE PROBLEM: in alt screen, the cursor position is in the hands of the proc handling the alt screen, for neovim to ensure `hidden` cursor if shown by horizontal scrolling doesn't make sense. But when we are in ~MODE_TERMINAL, then we care about whether the cursor is visible
- PROBLEM: Even if we have a method to not allow horizontal scrolling only when alt screen is active and the curor is not visible, when we get out in any other mode including normal or etc, we still can't change the visibility of cursor.
  - Sol: stash@{0}: WIP on term: 1458a784e3 test(terminal): resizing does not scroll horizontally

- Callback: `drawscreen.c:277`
void screen_resize(int width, int height)
  void win_new_screen_cols(void)
    void win_new_screen_cols(void)
      void win_reconfig_floats(void)
        void win_config_float(win_T *wp, WinConfig fconfig)
          void win_set_inner_size(win_T *wp, bool valid_cursor)
            void validate_cursor(win_T *wp)
              curs_columns(wp, true);

- WHAT IS VIRTUAL EDIT? `bool virtual_active(win_T *wp)`
Virtual edit is allowing edits in the buffer which is not possible to edit in text!
- examples: cursor b/w tab, cursor 1 char left to last char of file(I've been there :D)!

- `terminal_check_cursor`:
curwin->w_cursor.col = MAX(0, term->cursor.col + win_col_off(curwin) + off);
```c
win_col_off
  return ((wp->w_p_nu /* int number?: not set */ || wp->w_p_rnu /* int relative number: not set */ 
            || *wp->w_p_stc /* status col: idk */ != NUL)
      ? (number_width(wp) + (*wp->w_p_stc == NUL)) : 0) // Assumed false -> 0
  + ((wp != cmdwin_win) ? 0 : 1) // -> 0 
  + win_fdccol_count(wp) // fold cols to display -> 0
  + (wp->w_scwidth * SIGN_WIDTH); // sign col width * 2 -> (?)
 ```
curwin->w_cursor.coladd = 0;
mb_check_adjust_col(curwin);

colsadvance: -- posibly not an issue (commenting it out doesn't change behavior, altho it does but that because of the change in w_cursor)
-- with no coladv, we get horizontal scrolling of 1 to 2 chars
-- with coladv its like half the viewport width after resize!
getvcol:
  return coladvance2(wp, pos [wp->w_cursor], false, true, wcol);

- well, the case (may_scroll && !cline_fold) is gonna be default (because anything i change in cursor pos or whatever)
actually does not change cline_fold or may_scroll, so, well waste of 2 weeks
but:
```
# Without coladv (old method)                 # With coladv: (off by 8?)
w_leftcol = 0                                 w_leftcol = 0
siso = 0                                      siso = 0
startcol = 142                                startcol = 142
endcol = 142                                  endcol = 142
view_width = 70                               view_width = 70
off_left = 142                                off_left = 142
off_right = 73                                off_right = 73
diff = 73                                     diff = 73
w_wcol = 142                                  w_wcol = 142
extra = 0                                     extra = 0
width1 / 2 = 35                               width1 / 2 = 35
new_leftcol = 107                             new_leftcol = 107

w_leftcol = 107                               w_leftcol = 107
siso = 0                                      siso = 0
startcol = 61                                 startcol = 69
endcol = 61                                   endcol = 69
view_width = 70                               view_width = 70
off_left = -46                                off_left = -38
off_right = -115                              off_right = -107
diff = 46                                     diff = 38
w_wcol = 61                                   w_wcol = 69
extra = 0                                     extra = 0
width1 / 2 = 35                               width1 / 2 = 35
new_leftcol = 26                              new_leftcol = 34

w_leftcol = 26
siso = 0
startcol = 2
endcol = 2
view_width = 70
off_left = -24
off_right = -93
diff = 24
w_wcol = 2
extra = 0
width1 / 2 = 35
new_leftcol = 2
```

- BUG: cur_pos.col that is used to getvvcol in curs_col is getting >
  w_view_width of window setting it <= w_view_width gets shit done.
Why is it getting > tho that is an issue...
> who is setting cursor.col to 71? (not getvvcol)

`Sus:`
INF 2026-01-13T13:19:22.348 nvim.57194.0 terminal_check_cursor:860: before cursor.col: 9 col
INF 2026-01-13T13:19:22.348 nvim.57194.0 coladvance:80: before coladv: 35 wcol
INF 2026-01-13T13:19:22.348 nvim.57194.0 coladvance:82: after  coladv: 35 wcol
INF 2026-01-13T13:19:22.348 nvim.57194.0 coladvance:83: cursor.col: 71 col
INF 2026-01-13T13:19:22.348 nvim.57194.0 terminal_check_cursor:863: after  coladv: 35 wcol
INF 2026-01-13T13:19:22.348 nvim.57194.0 terminal_check_cursor:864: cursor.col: 71 col

`yup:`
INF 2026-01-13T13:21:10.795 nvim.59029.0 terminal_check_cursor:843: TERMINAL CURSOR CHECK
INF 2026-01-13T13:21:10.795 nvim.59029.0 coladvance:80: before cursor.col: 69 col
INF 2026-01-13T13:21:10.795 nvim.59029.0 coladvance:81: before coladv: 35 wcol
INF 2026-01-13T13:21:10.795 nvim.59029.0 coladvance:83: after  coladv: 35 wcol
INF 2026-01-13T13:21:10.795 nvim.59029.0 coladvance:84: cursor.col: 71 col

**GUILTY**:
```c /src/nvim/cursor.c:82
int rc = getvpos(wp, &wp->w_cursor, wcol);
```

> Why is w_cursor.lnum changing on horizontal resize?

INF 2026-01-21T15:14:41.947 nvim.38231.0 ml_get_buf_impl:1914: Invalif lnum last req: 34, cur 35
INF 2026-01-21T15:14:41.947 nvim.38231.0 ml_get_buf_impl:1914: Invalif lnum last req: 35, cur 36
INF 2026-01-21T15:14:41.947 nvim.38231.0 ml_get_buf_impl:1914: Invalif lnum last req: 36, cur 37
INF 2026-01-21T15:14:41.947 nvim.38231.0 ml_get_buf_impl:1914: Invalif lnum last req: 37, cur 38
DBG 2026-01-21T15:14:41.947 nvim.38231.0 inbuf_poll:514: blocking... events=false
INF 2026-01-21T15:14:41.947 nvim.38231.0 validate_cursor:646: VALIDATE CURSOR!
INF 2026-01-21T15:14:41.947 nvim.38231.0 curs_columns:825: cursor pos: 142
INF 2026-01-21T15:14:41.947 nvim.38231.0 getvvcol:636: col before = 142
INF 2026-01-21T15:14:41.947 nvim.38231.0 getvvcol:658: col after = 142
INF 2026-01-21T15:14:41.947 nvim.38231.0 curs_columns:895: w_leftcol = 107
INF 2026-01-21T15:14:41.947 nvim.38231.0 curs_columns:896: siso = 0
INF 2026-01-21T15:14:41.947 nvim.38231.0 curs_columns:897: startcol = 142
INF 2026-01-21T15:14:41.947 nvim.38231.0 curs_columns:898: endcol = 142
INF 2026-01-21T15:14:41.947 nvim.38231.0 curs_columns:899: view_width = 70
DBG 2026-01-21T15:14:41.947 nvim.38231.0 UI: raw_line (+42 times...)
DBG 2026-01-21T15:14:41.947 nvim.38231.0 UI: win_viewport
DBG 2026-01-21T15:14:41.947 nvim.38231.0 UI: grid_cursor_goto
DBG 2026-01-21T15:14:41.947 nvim.38231.0 UI: flush
INF 2026-01-21T15:14:41.947 nvim.38231.0 terminal_check:906: TERMINAL CHECK!
INF 2026-01-21T15:14:41.947 nvim.38231.0 terminal_check_cursor:843: TERMINAL CURSOR CHECK
INF 2026-01-21T15:14:41.947 nvim.38231.0 coladvance:80: before cursor.col: 142 col
INF 2026-01-21T15:14:41.947 nvim.38231.0 coladvance:81: before coladv: 35 wcol
INF 2026-01-21T15:14:41.947 nvim.38231.0 coladvance:82: w_cursor lnum: 33
INF 2026-01-21T15:14:41.947 nvim.38231.0 ml_get_buf_impl:1914: Invalif lnum last req: 38, cur 33

---

INF 2026-01-21T15:09:00.929 nvim.35013.0 ml_get_buf_impl:1915: Invalif lnum last req: 0, cur 35
INF 2026-01-21T15:09:00.929 nvim.35013.0 ml_get_buf_impl:1915: Invalif lnum last req: 0, cur 36
INF 2026-01-21T15:09:00.929 nvim.35013.0 ml_get_buf_impl:1915: Invalif lnum last req: 0, cur 37
INF 2026-01-21T15:09:00.929 nvim.35013.0 ml_get_buf_impl:1915: Invalif lnum last req: 0, cur 38 => 38 rightfully
DBG 2026-01-21T15:09:00.929 nvim.35013.0 inbuf_poll:514: blocking... events=false
INF 2026-01-21T15:09:00.929 nvim.35013.0 validate_cursor:646: VALIDATE CURSOR!
INF 2026-01-21T15:09:00.929 nvim.35013.0 curs_columns:825: cursor pos: 142
INF 2026-01-21T15:09:00.929 nvim.35013.0 getvvcol:636: col before = 142
INF 2026-01-21T15:09:00.929 nvim.35013.0 getvvcol:658: col after = 142
INF 2026-01-21T15:09:00.929 nvim.35013.0 curs_columns:895: w_leftcol = 107
INF 2026-01-21T15:09:00.929 nvim.35013.0 curs_columns:896: siso = 0
INF 2026-01-21T15:09:00.929 nvim.35013.0 curs_columns:897: startcol = 142
INF 2026-01-21T15:09:00.929 nvim.35013.0 curs_columns:898: endcol = 142
INF 2026-01-21T15:09:00.929 nvim.35013.0 curs_columns:899: view_width = 70
DBG 2026-01-21T15:09:00.929 nvim.35013.0 UI: raw_line (+42 times...)
DBG 2026-01-21T15:09:00.929 nvim.35013.0 UI: win_viewport
DBG 2026-01-21T15:09:00.929 nvim.35013.0 UI: grid_cursor_goto
DBG 2026-01-21T15:09:00.929 nvim.35013.0 UI: flush
DBG 2026-01-21T15:09:00.929 nvim.35013.0 handle_nvim_ui_set_focus:6689: RPC: ch 1: invoke nvim_ui_set_focus
INF 2026-01-21T15:09:00.929 nvim.35013.0 terminal_check:906: TERMINAL CHECK!
INF 2026-01-21T15:09:00.929 nvim.35013.0 terminal_check_cursor:843: TERMINAL CURSOR CHECK
INF 2026-01-21T15:09:00.929 nvim.35013.0 coladvance:80: before cursor.col: 142 col
INF 2026-01-21T15:09:00.929 nvim.35013.0 coladvance:81: before coladv: 35 wcol
INF 2026-01-21T15:09:00.929 nvim.35013.0 coladvance:82: w_cursor lnum: 33                       => 33?

NEXT:

INF 2026-01-21T15:09:00.930 nvim.35013.0 ml_get_buf_impl:1915: Invalif lnum last req: 0, cur 33
INF 2026-01-21T15:09:00.930 nvim.35013.0 ml_get_buf_impl:1915: Invalif lnum last req: 0, cur 34
INF 2026-01-21T15:09:00.930 nvim.35013.0 ml_get_buf_impl:1915: Invalif lnum last req: 0, cur 35
INF 2026-01-21T15:09:00.930 nvim.35013.0 ml_get_buf_impl:1915: Invalif lnum last req: 0, cur 36
INF 2026-01-21T15:09:00.930 nvim.35013.0 ml_get_buf_impl:1915: Invalif lnum last req: 0, cur 37
INF 2026-01-21T15:09:00.930 nvim.35013.0 ml_get_buf_impl:1915: Invalif lnum last req: 0, cur 38
INF 2026-01-21T15:09:00.930 nvim.35013.0 ml_get_buf_impl:1915: Invalif lnum last req: 0, cur 33  => What>>
DBG 2026-01-21T15:09:00.930 nvim.35013.0 UI: raw_line (+38 times...)
DBG 2026-01-21T15:09:00.930 nvim.35013.0 UI: win_viewport
DBG 2026-01-21T15:09:00.930 nvim.35013.0 UI: grid_cursor_goto
DBG 2026-01-21T15:09:00.930 nvim.35013.0 UI: mode_info_set
DBG 2026-01-21T15:09:00.930 nvim.35013.0 UI: mode_change
DBG 2026-01-21T15:09:00.930 nvim.35013.0 UI: mouse_on
DBG 2026-01-21T15:09:00.930 ui.35001   receive_msgpack:208: ch 3: parsing 3544 bytes from msgpack Stream: 0x559e0b2758b8
DBG 2026-01-21T15:09:00.930 nvim.35013.0 UI: flush
DBG 2026-01-21T15:09:00.930 nvim.35013.0 inbuf_poll:514: blocking... events=false
DBG 2026-01-21T15:09:00.930 nvim.35013.0 inbuf_poll:514: blocking... events=true
DBG 2026-01-21T15:09:00.937 nvim.35013.0 state_enter:97: input: K_EVENT
INF 2026-01-21T15:09:00.938 nvim.35013.0 terminal_check_cursor:843: TERMINAL CURSOR CHECK
INF 2026-01-21T15:09:00.938 nvim.35013.0 coladvance:80: before cursor.col: 77 col
INF 2026-01-21T15:09:00.938 nvim.35013.0 coladvance:81: before coladv: 35 wcol
INF 2026-01-21T15:09:00.938 nvim.35013.0 coladvance:82: w_cursor lnum: 33
INF 2026-01-21T15:09:00.938 nvim.35013.0 ml_get_buf_impl:1915: Invalif lnum last req: 0, cur 33
INF 2026-01-21T15:09:00.938 nvim.35013.0 coladvance2:114: lum=33

---

INF 2026-01-21T15:14:41.949 nvim.38231.0 ml_get_buf_impl:1914: Invalif lnum last req: 34, cur 35
INF 2026-01-21T15:14:41.949 nvim.38231.0 ml_get_buf_impl:1914: Invalif lnum last req: 35, cur 36
INF 2026-01-21T15:14:41.949 nvim.38231.0 ml_get_buf_impl:1914: Invalif lnum last req: 36, cur 37
INF 2026-01-21T15:14:41.949 nvim.38231.0 ml_get_buf_impl:1914: Invalif lnum last req: 37, cur 38
INF 2026-01-21T15:14:41.949 nvim.38231.0 ml_get_buf_impl:1914: Invalif lnum last req: 38, cur 33

```c /src/nvim/memline.c:1945
    buf->b_ml.ml_line_lnum = lnum; // set it to 33 (cur = lnum, check abv log)
```

---

- `virtual column`<br>
All* the info stored is in `byte offset`. To get the actual screen pixel col,
we use the virtual col (getvcol, getvvcol)

---

> w_cursor.lnum is updated because cursor.row is updated :(
Who is updating cursor.row?<br>

INF 2026-01-21T18:44:38.976 nvim.168875.0 terminal_check_cursor:843: TERMINAL CURSOR CHECK
INF 2026-01-21T18:44:38.976 nvim.168875.0 terminal_check_cursor:846: cursor.row=32
INF 2026-01-21T18:44:38.976 nvim.168875.0 terminal_check_cursor:847: sbcurrent=0
INF 2026-01-21T18:44:38.976 nvim.168875.0 terminal_check_cursor:859: curbuf->b_ml.ml_line_count=38
INF 2026-01-21T18:44:38.976 nvim.168875.0 terminal_check_cursor:860: w_cursor.lnum=33

**Ladies & Gentlemen, We got him**<br>
INF 2026-01-21T19:10:03.850 nvim.188467.0 curs_columns:825: cursor pos: 69
INF 2026-01-21T19:10:03.850 nvim.188467.0 curs_columns:826: lnum:: 38
INF 2026-01-21T19:10:03.850 nvim.188467.0 getvcol:580: vcol=67, head=0
INF 2026-01-21T19:10:03.850 nvim.188467.0 getvvcol:638: col before = 67
INF 2026-01-21T19:10:03.850 nvim.188467.0 getvvcol:660: col after = 67
INF 2026-01-21T19:10:03.850 nvim.188467.0 curs_columns:896: w_leftcol = 34
INF 2026-01-21T19:10:03.850 nvim.188467.0 curs_columns:897: siso = 0
INF 2026-01-21T19:10:03.850 nvim.188467.0 curs_columns:898: startcol = 67
INF 2026-01-21T19:10:03.850 nvim.188467.0 curs_columns:899: endcol = 67
INF 2026-01-21T19:10:03.850 nvim.188467.0 curs_columns:900: view_width = 143
INF 2026-01-21T19:10:03.850 nvim.188467.0 on_resize:2241: state pos row=24          => its libvterm on_resize (src/nvim/vterm/state:2242)
INF 2026-01-21T19:10:03.850 nvim.188467.0 term_movecursor:1355: old_row=37, new_row=24
DBG 2026-01-21T19:10:03.850 nvim.188467.0 UI: grid_clear

So, for the old code (without coladv), the coladv is actually needed, otherwise
the cursor pos in shell `:term` is not properly placed.
Another thing, for that old code with only 2 w_leftcol, this could've resolved:
```c src/nvim/vterm/screen:572
  new_row= 0;
  ILOG("VT_BREAK");
```
But unfortunately, this still gives us same result with coladv

---

> The only thing now to resolve is why coladv has col > wcol <br>
`INF 2026-01-21T20:39:14.303 nvim.262313.0 coladvance2:165: wcol = 69, col = 70`

}}}

[ISSUE #13484](https://github.com/neovim/neovim/issues/13484) `messages`
{{{
- src/nvim/errors.h
- EXTERN const char bot_top_msg[] INIT(= N_("search hit BOTTOM, continuing at TOP"));
- src/nvim/globals.h
- EXTERN char *keep_msg INIT( = NULL);        // msg to be shown after redraw
- ETERN bool msg_hist_off INIT( = false);     // don't add messages to history
- master failed: E5113 (59 skipped, 1 failed)
- remove history = false; (not needed ig?)
- Warning at: T1751, T1749
- The `save_x` dance
- [PR -> #36961](https://github.com/neovim/neovim/pull/36961)
}}}

[ISSUE #14986]() `statusline` `defaults` `terminal`
{{{
- something like `assert_log` like `assert_exit_code` ?
Tests to return to: -- skibidi
- test/functional/ex_cmds/mksession_spec.lua
- test/functional/terminal/tui_spec.lua (:restart works when connecting to remote instance (with its own TUI))

- more changes:
test/functional/terminal/buffer_spec.lua `with closed nvim_open_term() channel`
had to make changes in `nvim_open_term` `buf->b_p_channel = chan->id` (consequences?)


- Docs issue:
helptags dont work in `:h vim.o` for gx<br>
`current: https://neovim.io/doc/user/helptag.html?tag=vim.o`<br>
`expected: https://neovim.io/doc/user/lua/#vim.o`

- TODO:
1. exit_code field in nvim_get_chan_info
2. this option passes oldtest:

        "%{% luaeval('package.loaded[''vim.term''] and vim.term.get_code() or ''''')%}",
3. channel.c channel_info_changed ? can use that?

-- NEW:
1. Issue with `:term exit 4`

-- WEll:
```c nvim/drawscreen.c
  if (buf->terminal && !terminal_running(buf->terminal)) {
    static char msg[sizeof("[Process exited ]") + NUMBUFLEN];
    snprintf(msg, sizeof msg, "[Process exited %d]", terminal_exitcode(buf->terminal));
    static VirtTextChunk chunk = { .text = msg, .hl_id = -1 };
    static DecorVirtText virt_text = {
      .priority = DECOR_PRIORITY_BASE,
      .pos = kVPosWinCol,
      .data.virt_text = { .items = &chunk, .size = 1 },
    };
    decor_range_add_virt(&decor_state, terminal_row(buf->terminal) + 1, 0,
                         terminal_row(buf->terminal) + 1, 0, &virt_text, false);
  }
```

- extmark_free_all(buf);
}}}

}}}

vim: foldmethod=marker
