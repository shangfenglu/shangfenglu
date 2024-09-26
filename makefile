# 设置根目录为当前工作目录
ROOT_DIR		:= $(PWD)
# 设置项目名称
PROJECT_NAME	:= usm-lt
# 设置可执行文件后缀
EXE_SUFFIX		:= exe

# 设置输出路径为根目录
OUTPUT_PATH		:= $(ROOT_DIR)

# 设置用户宏定义
USER_MACROS		:= 	_FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE

# 设置包含目录路径
INCS_PATH		:= $(ROOT_DIR)  
INCS_PATH		+= ./include

# 设置库目录路径
LIBS_PATH		:= ./	$(ROOT_DIR) 
# 设置链接的库
<<<<<<< HEAD
LIBS			:=  pthread dl m jpeg png
=======
LIBS			:=  pthread dl m ev jpeg png sqlite3
>>>>>>> 9d35f9f4b22d63cd218f0ce540b2aec1fdc12bb5


# 设置模块路径
MODULES_PATH	:= .
# 其他模块文件路径
MODULES_PATH    += lib


# 设置源代码路径
SRC_PATH		:= $(ROOT_DIR)

# 获取所有源代码目录
SRC_DIRS		:= $(addprefix $(SRC_PATH)/, $(MODULES_PATH))
# 获取所有源代码文件
SRC				:= $(foreach SRC_DIRS, $(SRC_DIRS), $(wildcard $(SRC_DIRS)/*.c))

# 将源代码文件转换为目标文件
OBJS			:= $(patsubst %.c,%.o,$(SRC))
# 设置最终输出文件
FINAL_OUTPUT	:= $(addsuffix .$(EXE_SUFFIX),$(addprefix $(OUTPUT_PATH)/,$(PROJECT_NAME)))

# 设置编译选项
CFLAGS		:= $(CFLAG_COMM)  -Wall 
CFLAGS		+= -g -fPIC #-Wno-deprecated-declarations

# 设置编译器
CXX				:= gcc

# 生成目标文件的规则
.SECONDARY :%.o $(OBJS) 

%.o:%.c
	# 编译源代码为目标文件
	$(CXX) $(CFLAGS) $(addprefix -D, $(USER_MACROS)) $(addprefix -I,$(INCS_PATH)) -c $< -o $@ -D__NOTDIR_FILE__=$(notdir $<)

%.$(EXE_SUFFIX):$(OBJS)
	# 链接目标文件生成可执行文件
	$(CXX) -O2 $(CFLAGS) -o $@  $(OBJS)  $(addprefix -L,$(LIBS_PATH))  $(addprefix -l,$(LIBS))  

# 编译所有目标文件并生成可执行文件的规则
build_all:$(FINAL_OUTPUT)

# 显示相关变量的值
test:
	@echo SRC_PATH:	$(SRC_PATH)
	@echo ROOT_DIR:	$(ROOT_DIR)
	@echo SRC_DIRS:	$(SRC_DIRS)
	@echo SRC:	$(SRC)
	@echo OBJS:	$(OBJS)
	@echo FINAL_OUTPUT:	$(FINAL_OUTPUT)
	@echo CFLAGS:	$(CFLAGS)
	@echo USER_MACROS: $(USER_MACROS)
	
# 清除生成的文件
clean:
	@rm -rf $(OBJS)
	rm -rf $(OUTPUT_PATH)/$(PROJECT_NAME).$(EXE_SUFFIX)

# 运行可执行文件
start:
	export LD_LIBRARY_PATH=. && ./$(PROJECT_NAME).$(EXE_SUFFIX)   -d 7 -t logs/debug.log





