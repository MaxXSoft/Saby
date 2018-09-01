export cc = clang++
export std = c++1z
export opt_level = 2
export build_dir = ./build/
export debug = false

ifeq ($(debug), true)
	debug_arg = -g
	opt_arg = 
else
	debug_arg = 
	opt_arg = -O$(opt_level)
endif

CC = $(cc) $(debug_arg) -std=$(std) $(opt_arg)

# directories
saby_dir = ./src/
def_dir = $(saby_dir)define/
front_dir = $(saby_dir)front/
back_dir = $(saby_dir)back/
llvm_dir = $(saby_dir)llvm/
util_dir = $(saby_dir)util/

# define
symbol_targets = $(def_dir)symbol/symbol.cpp
ssa_targets = $(def_dir)ssa/def_use.cpp $(def_dir)ssa/ssa.cpp
def_targets = $(symbol_targets) $(ssa_targets)

# front-end
lexer_targets = $(front_dir)lexer/lexer.cpp
parser_targets = $(front_dir)parser/parser.cpp
analyzer_targets = $(front_dir)analyzer/analyzer.cpp $(front_dir)analyzer/sema.cpp
front_targets = $(lexer_targets) $(parser_targets) $(analyzer_targets)

# back-end
irbuilder_targets = $(back_dir)irbuilder/irbuilder.cpp $(back_dir)irbuilder/genir.cpp
optimizer_targets = $(back_dir)optimizer/optimizer.cpp
back_targets = $(irbuilder_targets) $(optimizer_targets)

# LLVM back-end

# util
fs_targets = $(util_dir)fs/dir.cpp
util_targets = $(fs_targets)

# output
saby4llvm_out = $(build_dir)s4l

lexer_test_targets = $(lexer_targets) $(front_dir)lexer/lexer_test.cpp
lexer_test_out = $(build_dir)lexer

parser_test_targets = $(def_targets) $(front_targets) $(back_targets) $(util_targets) $(front_dir)parser/parser_test.cpp
parset_test_out = $(build_dir)parser

outs = $(lexer_test_out) $(parset_test_out)

.PHONY: all saby lexer parser clean clean_dbg

all: saby saby4llvm lexer parser

saby: parser

saby4llvm: $(parser_test_targets)
	$(CC) $(parser_test_targets) -o $(saby4llvm_out) -DLLVM_BACKEND

lexer: $(lexer_test_targets)
	$(CC) $(lexer_test_targets) -o $(lexer_test_out)

parser: $(parser_test_targets)
	$(CC) $(parser_test_targets) -o $(parset_test_out)

clean: clean_dbg
	(rm $(outs)) || true

clean_dbg:
	(-rm -r $(build_dir)*.dSYM) || true
