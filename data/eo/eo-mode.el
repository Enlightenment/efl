;;; eo-mode-el -- Major mode for editing Eo (eolian) files

;; Author: Gustavo Sverzut Barbieri <barbieri@profusion.mobi>
;; Created: 2016-07-29
;; Keywords: EFL Eo major-mode

;; Copyright (C) 2016 Gustavo Sverzut Barbieri <barbieri@profusion.mobi>

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

;; To enable for *.eo and *.eot files, add to ~/.emacs:
;;
;;    (add-to-list 'load-path "SOMEPATH/.emacs.d/site-packages/eo-mode.el")
;;    (add-to-list 'auto-mode-alist '("\\.eo$\\|\\.eot$" . eo-mode))
;;    (autoload 'eo-mode "eo-mode")


(defvar eo-mode-hook nil)

(defconst eo-font-lock-units
  (eval-when-compile
    (list
     (list (concat
            "^\\<"
            (regexp-opt
             '(
               "abstract"
               "class"
               "mixin"
               "interface"
               "type"
               "const"
               "var"
               "struct"
               "enum"
               "import"
               ) t) "\\>\\(\\(\\s-+@"
                 "\\(extern\\|free([^)]+)\\)"
                 "\\)+\\|\\)\\s-+\\<\\([a-zA-Z_][a-zA-Z0-9_.]*\\)\\>")
           '(1 font-lock-keyword-face)
           '(2 font-lock-type-face)
           '(5 font-lock-function-name-face)
           )))
  "Toplevel Units")

(defconst eo-font-lock-blocks
  (eval-when-compile
    (append eo-font-lock-units
            (list
             (list (concat
                    "^\\s-+"
                    (regexp-opt
                     '(
                       "methods"
                       "events"
                       "implements"
                       "constructors"
                       "params"
                       "keys"
                       "values"
                       ) t) "\\s-*{")
                   '(1 font-lock-builtin-face)
                   ))))
  "blocks")

(defconst eo-font-lock-blocks-with-modifiers
  (eval-when-compile
    (append eo-font-lock-blocks
            (list
             (list (concat
                    "^\\s-+"
                    (regexp-opt
                     '(
                       "get"
                       "set"
                       ) t)
                    "\\(\\s-+@"
                    (regexp-opt
                     '(
                       "virtual_pure"
                       ) t)
                    "\\|\\)\\s-*{")
                   '(1 font-lock-builtin-face)
                   '(2 font-lock-type-face)
                   ))))
  "blocks")

(defconst eo-font-lock-constants
  (eval-when-compile
    (append eo-font-lock-blocks-with-modifiers
            (list
             (list (concat
                    "\\<"
                    (regexp-opt
                     '(
                       "null"
                       "true"
                       "false"
                       ) t) "\\>")
                   '(1 font-lock-constant-face)
                   ))))
  "Constants")

(defconst eo-font-lock-event
  (eval-when-compile
    (append eo-font-lock-constants
            (list
             (list (concat
                    "^\\s-+\\([a-zA-Z][a-zA-Z0-9_,]*\\)\\(\\s-+@"
                    (regexp-opt
                     '(
                       "private"
                       "protected"
                       "beta"
                       "hot"
                       ) t) "\\|\\)\\s-*\\(:\\|;\\)")
                   '(1 font-lock-variable-name-face)
                   '(2 font-lock-type-face)
                   ))))
  "event")

(defconst eo-font-lock-param
  (eval-when-compile
    (append eo-font-lock-event
            (list
             (list (concat
                    "^\\s-+\\(@"
                    (regexp-opt
                     '(
                       "in"
                       "out"
                       "inout"
                       ) t)
                    "\\s-+\\|\\)\\([a-zA-Z][a-zA-Z0-9_]*\\)"
                    "\\s-*:[^@]+*\\(\\(\\s-+@"
                    (regexp-opt
                     '(
                       "nonull"
                       "nullable"
                       "optional"
                       ) t)
                    "\\)+\\|\\)\\s-*;"
                    )
                   '(1 font-lock-type-face)
                   '(3 font-lock-variable-name-face)
                   '(4 font-lock-type-face)
                   ))))
  "parameters")


(defconst eo-font-lock-return
  (eval-when-compile
    (append eo-font-lock-param
            (list
             (list (concat
                    "^\\s-+return\\s-*:[^@]+*\\(\\(\\s-+@"
                    (regexp-opt
                     '(
                       "warn_unused"
                       ) t)
                    "\\)+\\|\\)\\s-*;"
                    )
                   '(1 font-lock-type-face)
                   ))))
  "return")

(defconst eo-font-lock-default-implementation
  (eval-when-compile
    (append eo-font-lock-return
            (list
             (list (concat
                    "^\\s-+"
                    (regexp-opt
                     '(
                       "class.constructor"
                       "class.destructor"
                       ) t)
                    "\\s-*;"
                    )
                   '(1 font-lock-builtin-face)
                   ))))
  "Default Implementation")

(defconst eo-font-lock-implementation
  (eval-when-compile
    (append eo-font-lock-default-implementation
            (list
             (list (concat
                    "^\\s-+"
                    "\\(@\\(auto\\|empty\\)\\s-+\\|\\)"
                    "\\([a-zA-Z.][a-zA-Z0-9_.]*\\)"
                    "\\s-*;"
                    )
                   '(1 font-lock-type-face)
                   '(3 font-lock-variable-name-face)
                   ))))
  "Implementation")

(defconst eo-font-lock-property
  (eval-when-compile
    (append eo-font-lock-implementation
            (list
             (list (concat
                    "^\\s-+\\(@property\\)\\s-+"
                    "\\([a-zA-Z][a-zA-Z0-9_]*\\)\\(\\(\\s-+@"
                    (regexp-opt
                     '(
                       "protected"
                       "class"
                       "virtual_pure"
                       "part"
                       ) t)
                    "\\)+\\|\\)\\s-*{"
                    )
                   '(1 font-lock-type-face)
                   '(2 font-lock-variable-name-face)
                   '(3 font-lock-type-face)
                   ))))
  "Property method")

(defconst eo-font-lock-method
  (eval-when-compile
    (append eo-font-lock-property
            (list
             (list (concat
                    "^\\s-+\\([a-zA-Z][a-zA-Z0-9_]*\\)\\(\\(\\s-+@"
                    (regexp-opt
                     '(
                       "protected"
                       "const"
                       "class"
                       "virtual_pure"
                       ) t)
                    "\\)+\\|\\)\\s-*{"
                    )
                   '(1 font-lock-variable-name-face)
                   '(2 font-lock-type-face)
                   ))))
  "Method")


(defconst eo-font-lock-fields
  (eval-when-compile
    (append eo-font-lock-method
            (list
             (list "^\\s-+\\<\\([a-zA-Z0-9][a-zA-Z0-9_,]*\\)\\>:"
                   '(1 font-lock-variable-name-face)
                   ))))
  "Fields & Events without modifiers")

(defconst eo-font-lock-complex-type
  (eval-when-compile
    (append eo-font-lock-fields
            (list
             (list (concat
                    "\\<"
                    (regexp-opt
                     '(
                       "accessor"
                       "array"
                       "iterator"
                       "hash"
                       "list"
                       ) t)
                    "\\>\\s-*<"
                    )
                   '(1 font-lock-builtin-face)
                   ))))
  "Complex types, C++ template style.")

(defconst eo-font-lock-expression-functions
  (eval-when-compile
    (append eo-font-lock-complex-type
            (list
             (list (concat
                    "\\<"
                    (regexp-opt
                     '(
                       "own"
                       "const"
                       "free"
                       ) t)
                    "\\>\\s-*("
                    )
                   '(1 font-lock-builtin-face)
                   ))))
  "Expression Functions")

(defconst eo-font-lock-known-types
  (eval-when-compile
    (append eo-font-lock-expression-functions
            (list
             (list (concat
                    "\\<"
                    (regexp-opt
                     '(
                       "byte"
                       "ubyte"
                       "char"
                       "short"
                       "ushort"
                       "int"
                       "uint"
                       "long"
                       "ulong"
                       "llong"
                       "ullong"
                       "int8"
                       "uint8"
                       "int16"
                       "uint16"
                       "int32"
                       "uint32"
                       "int64"
                       "uint64"
                       "int128"
                       "uint128"
                       "size"
                       "ssize"
                       "intptr"
                       "uintptr"
                       "ptrdiff"
                       "time"
                       "float"
                       "double"
                       "bool"
                       "void"
                       "void_ptr"
                       ) t)
                    "\\>"
                    )
                   '(1 font-lock-builtin-face)
                   ))))
  "Expression Functions")

(defgroup eo nil
  "EFL's Eo formatting."
  :prefix "eo-"
  :group 'faces)

(defface eo-docstring-face
  '((t :inherit font-lock-string-face))
  "Face used to highlight documentation strings."
  :group 'eo)

(defconst eo-font-lock-docstring
  (eval-when-compile
    (append eo-font-lock-known-types
            (list
             (list "\\(\\[\\[.*\\]\\]\\)"
                   '(1 'eo-docstring-face t) ;; t = overrides comment existing formatting
                   ))))
  "Documentation")

(defvar eo-font-lock eo-font-lock-docstring)

(defvar eo-mode-syntax-table
  (let ((eo-mode-syntax-table (make-syntax-table)))
    ;; [ is open and matches ], but [[ is a docstring starter
    (modify-syntax-entry ?[ "(]12c" eo-mode-syntax-table)
                         (modify-syntax-entry ?] ")[34c" eo-mode-syntax-table)
    eo-mode-syntax-table)
  "Syntax table for eo-mode")

(c-add-style
 "eo"
 '("gnu"
   (indent-tabs-mode . nil)
   (tab-width . 8)
   (c-basic-offset . 4)
   (c-backslash-column . 72)
   ))

(define-derived-mode eo-mode c-mode "Eo"
  "Major mode for editing Eo files"
  (interactive)
  (use-local-map eo-mode-map)
  (set-syntax-table eo-mode-syntax-table)
  (set (make-local-variable 'font-lock-defaults) '(eo-font-lock))
  (set (make-local-variable 'require-final-newline) t)
  (c-set-style "eo")
  (run-hooks 'eo-mode-hook)
  )

(provide 'eo-mode)
