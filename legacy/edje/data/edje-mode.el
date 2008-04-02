;;; edje-mode-el -- Major mode for editing Edje files

;; Author: Gustavo Sverzut Barbieri <barbieri@gmail.com>
;; Created: 2007-07-23
;; Keywords: Edje major-mode
;; Url: http://barbieri-playground.googlecode.com/svn/dot-files/edje-mode.el
;;      (if you find this file have problems, check that Url and request update)

;; Copyright (C) 2007 Gustavo Sverzut Barbieri <barbieri@gmail.com>

;; This program is free software; you can redistribute it and/or
;; modify it under the terms of the GNU General Public License as
;; published by the Free Software Foundation; either version 2 of
;; the License, or (at your option) any later version.

;; This program is distributed in the hope that it will be
;; useful, but WITHOUT ANY WARRANTY; without even the implied
;; warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
;; PURPOSE.  See the GNU General Public License for more details.

;; You should have received a copy of the GNU General Public
;; License along with this program; if not, write to the Free
;; Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
;; MA 02111-1307 USA

;;; Commentary:
;;
;; This mode is based on tutorial from Scott Andrew Borton:
;; http://two-wugs.net/emacs/mode-tutorial.html


(defvar edje-mode-hook nil)

(defun number-or-nil-to-string (v &optional default)
  (cond ((numberp v) (number-to-string v))
        ((stringp v) (if (string= v "") (number-to-string default) v))
        (t           (number-to-string default))))

(defun non-empty-string (s)
  (and (not (eq 'nil s))
       (not (string= "" s))))

(defun edje-new-program-action-signal-emit (source emission)
  "Insert new program SIGNAL_EMIT"
  (interactive "ssource: \nsemission: ")
  (insert
   (concat
    "               action: SIGNAL_EMIT \"" source "\" \"" emission "\";\n"
    )))

(defun edje-new-program-action-state-set (state value target)
  "Insert new program STATE_SET"
  (interactive "sstate: \nsvalue (0.0): \nstarget: ")
  (insert
   (concat
    "               action: STATE_SET \"" state "\" "
       (number-or-nil-to-string value 0.0) ";\n"
    "               target: \"" target "\";\n"
    )))

(defun edje-new-program-action (action)
  "Insert new program action"
  (interactive "saction: ")
  (setq action (upcase action))
  (cond ((string= action "STATE_SET")
         (edje-new-program-action-state-set "" 0.0 ""))
        ((string= action "SIGNAL_EMIT")
         (edje-new-program-action-signal-emit "" ""))
        ))

(defun edje-new-program (name signal source action)
  "Insert new program block"
  (interactive "sname: \nssignal: \nssource: \nsaction: ")
  (insert
   (concat
    "\n"
    "            program {\n"
    "               name: \"" name "\";\n"

    (if (non-empty-string signal)
        (concat "               signal: \"" signal "\";\n"))

    (if (non-empty-string source)
        (concat "               source: \"" source "\";\n"))
    ))

  (edje-new-program-action action)

  (insert
   (concat
    "            }\n"
    "\n"
    )))

(defun edje-new-desc-relative (x y &optional defx defy)
  "Insert new part description 'relative' line"
  (interactive "sx: \nsy: ")
  (insert
   (concat
    "                  relative: "
    (number-or-nil-to-string x defx) " "
    (number-or-nil-to-string y defy) ";\n"
    )))

(defun edje-new-desc-offset (x y &optional defx defy)
  "Insert new part description 'offset' line"
  (interactive "sx: \nsy: ")
  (insert
   (concat
    "                  offset: "
    (number-or-nil-to-string x defx) " "
    (number-or-nil-to-string y defy) ";\n"
    )))

(defun edje-new-desc-inherit (name val)
  "Insert new part description 'inherit' line"
  (interactive "sname: \nsvalue: ")
  (insert
   (concat
    "               inherit: \"" name "\" "
    (number-or-nil-to-string val 0.0) ";\n"
    )))

(defun edje-new-desc-text (font size text)
  "Insert new part description 'text' block"
  (interactive "sfont: \nssize: \nstext: ")
  (insert
   (concat
    "               text {\n"
    "                  font: \"" font "\";\n"
    "                  size: " (number-or-nil-to-string size) ";\n"
    "                  text: \"" text "\";\n"
    "               }\n"
    )))

(defun edje-new-desc-image (name)
  "Insert new part description 'image' block"
  (interactive "sname: ")
  (insert
   (concat
    "               image {\n"
    "                  normal: \"" name "\";\n"
    "               }\n"
    )))

(defun edje-new-desc-color (r g b a &optional defr defg defb defa)
  "Insert new part description 'color' line"
  (interactive "sred: \nsgreen: \nsblue: \nsalpha: ")
  (insert
   (concat
    "               color: "
    (number-or-nil-to-string r defr) " "
    (number-or-nil-to-string g defg) " "
    (number-or-nil-to-string b defb) " "
    (number-or-nil-to-string a defa) ";\n"
    )))

(defun edje-new-desc (name val &optional
                           r1_rx r1_ry
                           r2_rx r2_ry
                           r1_ox r1_oy
                           r2_ox r2_oy
                           part_type)
  "Insert new part description block"
  (interactive "sName: \nsValue: ")
  (insert
   (concat
    "            description {\n"
    "               state: \"" name "\" " (number-or-nil-to-string val 0.0) ";\n"))
  (if (string= part_type "RECT") (edje-new-desc-color 255 255 255 255))
  (insert "               rel1 {\n")
  (edje-new-desc-relative r1_rx r1_ry 0.0 0.0)
  (edje-new-desc-offset r1_ox r1_oy 0 0)
  (insert
   (concat
    "               }\n"
    "               rel2 {\n"
    ))
  (edje-new-desc-relative r2_rx r2_ry 1.0 1.0)
  (edje-new-desc-offset r2_ox r2_oy -1 -1)
  (insert "               }\n")
  (cond ((string= part_type "IMAGE") (edje-new-desc-image ""))
        ((string= part_type "TEXT") (edje-new-desc-text "" 10 "contents"))
        )
  (insert "            }\n")
  )

(defun edje-new-part (name type &optional
                           r1_rx r1_ry
                           r2_rx r2_ry
                           r1_ox r1_oy
                           r2_ox r2_oy)
  "Insert new part"
  (interactive "sName: \nsType: ")
  (setq type (upcase type))
  (insert
   (concat
    "\n"
    "         part {\n"
    "            name: \"" name "\";\n"
    "            type: " type ";\n"
    "            mouse_events: 0;\n"
    ))
  (edje-new-desc "default" 0.0 r1_rx r1_ry r2_rx r2_ry r1_ox r1_oy r2_ox r2_oy type)
  (insert
   (concat
    "         }\n"
    )))

(defun edje-setup-compile ()
  (set (make-local-variable 'compile-command)
       (concat "edje_cc " (buffer-file-name))
  ))

(defun edje-cc ()
  "Runs edje_cc with current buffer."
  (interactive)
  (compile (edje-setup-compile)))

(defvar edje-mode-map
  (let ((edje-mode-map (make-sparse-keymap)))
    (define-key edje-mode-map "\C-j" 'newline-and-indent)
    (define-key edje-mode-map "\C-cp" 'edje-new-part)
    (define-key edje-mode-map "\C-cd" 'edje-new-desc)
    (define-key edje-mode-map "\C-cr" 'edje-new-desc-relative)
    (define-key edje-mode-map "\C-co" 'edje-new-desc-offset)
    (define-key edje-mode-map "\C-ch" 'edje-new-desc-inherit)
    (define-key edje-mode-map "\C-cc" 'edje-new-desc-color)
    (define-key edje-mode-map "\C-ci" 'edje-new-desc-image)
    (define-key edje-mode-map "\C-ct" 'edje-new-desc-text)
    (define-key edje-mode-map "\C-cg" 'edje-new-program)
    (define-key edje-mode-map "\C-ca" 'edje-new-program-action)
    (define-key edje-mode-map "\C-cs" 'edje-new-program-action-state-set)
    (define-key edje-mode-map "\C-ce" 'edje-new-program-action-signal-emit)
    edje-mode-map)
  "Keymap for Edje major mode")

(add-hook 'c-mode-hook 'edje-setup-compile)
(add-to-list 'auto-mode-alist '("\\.edc$" . edje-mode))

(defconst edje-font-lock-keywords-1
  (eval-when-compile
    (list
     (list (concat "[ \t]*\\<"
                   (regexp-opt
                    '(
                      "collections"
                      "data"
                      "description"
                      "dragable"
                      "fill"
                      "fonts"
                      "group"
                      "image"
                      "images"
                      "origin"
                      "part"
                      "parts"
                      "program"
                      "programs"
                      "rel1"
                      "rel2"
                      "script"
                      "spectra"
                      "style"
                      "styles"
                      "text"
                      ) t) "\\>\\([ \t]*{\\|\\.\\)")
           '(1 font-lock-function-name-face))

     ))
  "Major keywords")

(defconst edje-font-lock-keywords-2
  (eval-when-compile
    (append edje-font-lock-keywords-1
            (list
             (list
              (concat "^\\([ \t]*\\|[ \t]*[a-z]+\\.\\|\\)\\<"
                      (regexp-opt
                       '("action"
                         "after"
                         "alias"
                         "align"
                         "angle"
                         "aspect"
                         "aspect_preference"
                         "base"
                         "border"
                         "clip_to"
                         "collections"
                         "color"
                         "color2"
                         "color3"
                         "color_class"
                         "color_classes"
                         "confine"
                         "data"
                         "description"
                         "dragable"
                         "effect"
                         "elipsis"
                         "events"
                         "fill"
                         "fit"
                         "fixed"
                         "font"
                         "fonts"
                         "gradient"
                         "group"
                         "ignore_flags"
                         "image"
                         "images"
                         "in"
                         "inherit"
                         "item"
                         "max"
                         "middle"
                         "min"
                         "mouse_events"
                         "name"
                         "normal"
                         "offset"
                         "origin"
                         "part"
                         "parts"
                         "pointer_mode"
                         "precise_is_inside"
                         "program"
                         "programs"
                         "rel1"
                         "rel2"
                         "relative"
                         "repeat_events"
                         "signal"
                         "size"
                         "smooth"
                         "source"
                         "spectra"
                         "spectrum"
                         "spread"
                         "state"
                         "step"
                         "style"
                         "styles"
                         "tag"
                         "target"
                         "text"
                         "text_class"
                         "text_source"
                         "to"
                         "to_x"
                         "to_y"
                         "transition"
                         "tween"
                         "type"
                         "use_alternate_font_metrics"
                         "visible"
                         "x"
                         "y"
                         ) t) "\\>[ \t]*[:,]")
              '(2 font-lock-keyword-face))
             )))
  "Minor keywords")

(defconst edje-font-lock-keywords-3
  (eval-when-compile
    (append edje-font-lock-keywords-2
            (list
             (list
              (concat "\\<"
                      (regexp-opt
                       '(; image options (st_images_image)
                         "RAW"
                         "COMP"
                         "LOSSY"
                         "USER"
                         ; part types (st_collections_group_parts_part_type)
                         "NONE"
                         "RECT"
                         "TEXT"
                         "IMAGE"
                         "SWALLOW"
                         "TEXTBLOCK"
                         "GRADIENT"
                         "GROUP"
                         ; ignore flags (st_collections_group_parts_part_ignore_flags)
                         ;"NONE"
                         "ON_HOLD"
                         ; pointer mode (st_collections_group_parts_part_pointer_mode)
                         "AUTOGRAB"
                         "NOGRAB"
                         ; aspect (st_collections_group_parts_part_description_aspect_preference)
                         "NONE"
                         "VERTICAL"
                         "HORIZONTAL"
                         "BOTH"
                         ; text effect (st_collections_group_parts_part_effect)
                         "NONE"
                         "PLAIN"
                         "OUTLINE"
                         "SOFT_OUTLINE"
                         "SHADOW"
                         "SOFT_SHADOW"
                         "OUTLINE_SHADOW"
                         "OUTLINE_SOFT_SHADOW"
                         "FAR_SHADOW"
                         "FAR_SOFT_SHADOW"
                         "GLOW"
                         ; image fill (st_collections_group_parts_part_description_fill_type)
                         "SCALE"
                         "TILE"
                         ; program action (st_collections_group_programs_program_action)
                         "STATE_SET"
                         "ACTION_STOP"
                         "SIGNAL_EMIT"
                         "DRAG_VAL_SET"
                         "DRAG_VAL_STEP"
                         "DRAG_VAL_PAGE"
                         "SCRIPT"
                         ; program transition (st_collections_group_programs_program_transition)
                         "LINEAR"
                         "SINUSOIDAL"
                         "ACCELERATE"
                         "DECELERATE"
                         ) t) "\\>")
              '(1 font-lock-builtin-face))
             )))
  "Enumerate values")

(defconst edje-font-lock-keywords-4
  (eval-when-compile
    (append edje-font-lock-keywords-3
            (list
             (list
              (concat "[ \t]*#"
                      (regexp-opt
                       '("if"
                         "ifdef"
                         "ifndef"
                         "define"
                         "else"
                         "endif"
                         "include"
                         "undef") t) "[ \t]*")
              '(1 font-lock-builtin-face))
             )))
  "CPP directives")

(defconst edje-font-lock-keywords-5
  (eval-when-compile
    (append edje-font-lock-keywords-4
            (list
             (list "[ \t]*#undef[ \t]+\\([a-zA-Z_][a-zA-Z0-9_]*\\)"
                   '(1 font-lock-variable-name-face))
             (list "[ \t]*#define[ \t]+\\([a-zA-Z_][a-zA-Z0-9_]*\\)("
                   '(1 font-lock-function-name-face))
             (list "[ \t]*#define[ \t]+\\([a-zA-Z_][a-zA-Z0-9_]*\\)"
                   '(1 font-lock-variable-name-face))
             )))
  "CPP directives that define constants")


(defvar edje-font-lock-keywords edje-font-lock-keywords-5)

(defvar edje-mode-syntax-table
  (let ((edje-mode-syntax-table (make-syntax-table)))
    ; This is added so entity names with underscores can be more easily parsed
    (modify-syntax-entry ?_ "w" edje-mode-syntax-table)
    (modify-syntax-entry ?/ ". 124b" edje-mode-syntax-table)
    (modify-syntax-entry ?* ". 23" edje-mode-syntax-table)
    (modify-syntax-entry ?\n "> b" edje-mode-syntax-table)

    edje-mode-syntax-table)
  "Syntax table for edje-mode")

(c-add-style
 "edje"
 '("gnu"
   (indent-tabs-mode . nil)
   (tab-width . 8)
   (c-basic-offset . 3)
   (c-backslash-column . 72)
   (c-hanging-braces-alist  .
                            ((block-open after)
                             (brace-list-open after)
                             (substatement-open after))
                            )
   (c-offsets-alist         .
                            ((statement-block-intro . +)
                             (defun-open            . 0)
                             (substatement-open     . 0)
                             (defun-block-intro     . +)
                             (block-open            . 0)
                             (label                 . +)
                             ))))


(define-derived-mode edje-mode c-mode "Edje"
  "Major mode for editing Edje files"
  (interactive)
  (use-local-map edje-mode-map)
  (set-syntax-table edje-mode-syntax-table)
  (set (make-local-variable 'font-lock-defaults) '(edje-font-lock-keywords))
  (set (make-local-variable 'require-final-newline) t)
  (c-set-style "edje")
  (run-hooks 'edje-mode-hook)
  )

(provide 'edje-mode)

;;; edje-mode.el ends here
