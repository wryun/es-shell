(defvar es-mode-hook nil)
(defvar es-mode-map
  (let ((es-mode-map (make-keymap)))
	(define-key es-mode-map "\C-j" 'newline-and-indent)
	es-mode-map)
    "Keymap for es major mode")

(add-to-list 'auto-mode-alist '("\\.es\\'" . es-mode))

(defvar es-font-lock-keywords
  '(
    ("#.*$" . font-lock-comment-face)
    ("'[^\']*'" font-lock-variable-face)
    
    ("\\<\\(let\\|if\\|for\\|while\\|fn\\)\\>" . font-lock-keyword-face)
    ("\\<\\(access\\|break\\|catch\\|cd\\|echo\\|eval\\|exec\\|exit\\|false\\|forever\\|fork\\|if\\|limit\\|newpgrp\\|result\\|return\\|throw\\|time\\|true\\|umask\\|unwind-protect\\|var\\|vars\\|wait\\|whatis\\|while\\|%read\\)\\>" . font-lock-builtin-face)
    
    ("\"[^\"]*\"" 0 font-lock-string-face t)
    ("`{[^}]*}" 0 font-lock-variable-name-face t)
    ("\\<-\\w*\\>" 0 font-lock-reference-face t)
    ("\$\\w*" 0 font-lock-reference-face t)
    ))

(defvar es-mode-syntax-table
  (let ((es-mode-syntax-table (make-syntax-table)))
    (modify-syntax-entry ?_ "w" es-mode-syntax-table)
    (modify-syntax-entry ?- "w" es-mode-syntax-table)
    (modify-syntax-entry ?. "w" es-mode-syntax-table)
    es-mode-syntax-table)
    "Syntax table for es-mode")

(defun es-indent-line ()
  "Indent current line as es code"
  (interactive)
  (beginning-of-line))

(define-derived-mode es-mode fundamental-mode "es"
  "Major mode for editing Extensible Shell scripts."
  (set (make-local-variable 'font-lock-defaults) '(es-font-lock-keywords))
  (set (make-local-variable 'indent-line-function) 'es-indent-line))

(provide 'es-mode)
