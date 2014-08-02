/*
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef LISP_H_
#define LISP_H_

#include <stdio.h>

#define C_BOOL int
#define C_TRUE 1
#define C_FALSE 0

enum type {
  INTEGER, /* LISPINT * */
  STRING, /* char * */
  SYMBOL, /* char * */
  LIST, /* struct lisp_object * or NULL for empty list. */
  FUNCTION,	/* struct lisp_function * */
  BUILTIN, /* struct lisp_object* (*func)(struct lisp_object*) */
  T_TYPE /* NULL */
};

struct lisp_object {
  void *data;

  struct lisp_object *prev;
  struct lisp_object *next;

  enum type obj_type;

  C_BOOL quoted;
};

/* Type conversion macros */
#define TOLINT(n) (*((LISPINT*)( (n) ->data)))
#define TOSTR(n) ((char*)( (n) ->data))
#define SYM_NAME(n) ((char*)( (n) ->data))
#define TOBUILTIN(n) ((struct lisp_builtin*)( (n) ->data))
#define TOFUNC(n) ((struct lisp_function*)( (n) ->data))

extern struct lisp_object *t;
extern struct lisp_object *nil;

struct symbol {
  char *symbol_name;
  struct lisp_object *value;
  C_BOOL constant;
};

extern struct symbol *symbol_table;
extern int symbol_table_counter;

/*
 * Initialize the Lisp interpreter symbol table and builtins.
 */
void lisp_initialize();

/*
 * The C implementation of the Lisp `read` function.
 * Input: FILE *input - Where to read the text for the reader from. NULL means stdin.
 * Output: lisp_object* - The root of the unevaluated objects returned.
 */
struct lisp_object *c_read(FILE *input);

/*
 * The C implementation of the Lisp `eval` function.
 * Input: lisp_object *root - The root of the Lisp object to be evaluated. Usually output from read()
 * Output: lisp_object* - The result of the Lisp object's evaluation.
 */
struct lisp_object *c_eval(struct lisp_object *root);


/*
 * Gets a new symbol from the symbol table and registers it.
 * Note: This function may allocate more memory for the symbol object.
 */
struct symbol *get_new_symbol();

/*
 * The C implementation of the Lisp `print` function.
 * Input: lisp_object *obj - The object to print.
 * Output: nil.
 */
struct lisp_object *c_print(struct lisp_object *obj);

int list_length(struct lisp_object *head);


/*
 * Does a deep copy of a lisp_object structure. This should be avoided if possible.
 * Input: lisp_object *obj - An object to be deep copied
 * Output: lisp_object* - A deep copy of obj.
 */
struct lisp_object *lisp_object_deep_copy(struct lisp_object *obj);

/*
 * Defines the way parameters are restricted.
 */
enum paramspec {
  VAR_FIXED = 0x0001, /* The number of parameters must be equal to numparams */
  VAR_MIN = 0x0010, /* Defines the function as having a minimum number of arguments, as defined by numparams */
  VAR_MAX = 0x0100, /* Defines the function as having a maximum number of arguments, as defined by numparams */
  UNEVAL_ARGS = 0x1000 /* Do not evaluate the arguments to this function. */
};

/*
 * Defines a builtin function. Only called from C.
 * Input:
 * char *symbol_name - The name to be used to call this function in Lisp.
 * enum paramspec spec - Defines the way parameters are restricted.
 * int numparams - Number of parameters - Exact interpretation depends on spec. See above paramspec docs.
 * lisp_object* (*func)(lisp_object*) - The C function you are defining. Takes a lisp_object which is always a
 *                                      LIST of the passed-in parameters. Returns a lisp_object which will be
 *                                      returned back to the Lisp code.
 */
void define_builtin_function(char *symbol_name, enum paramspec spec, int numparams,
			     struct lisp_object* (*func)(struct lisp_object*));

struct symbol *symbol_lookup(char *key);
struct lisp_object *symbol_value(char *key);

/*
 * Set the error and the error flag
 */
void set_error(char *error, ...);

 /*
  * Gets the current error or NULL if no error has occurred.
  */
char *get_error();

/*
 * Returns C_TRUE if an error has occurred.
 */
C_BOOL has_error();

/*
 * Allocated by the functions generated by the DEFUN macro to represent a builtin.
 */
struct lisp_builtin {
  struct lisp_object* (*func)(struct lisp_object*);
  enum paramspec spec;
  int params;
};

/*
 * Defines a non-builtin Lisp function.
 */
struct lisp_function {
  /* A list of SYMBOLs defining parameter names. */
  struct lisp_object *params;
  /* A cached value for the number of params (=list_length(params)) */
  int numparams;
  /* The head of the list of forms defining this fn */
  struct lisp_object *forms;
};

/* Macro to define a function from C. This requires that this be called in register_builtins() in lisp.c */
#define DEFUN(lisp_name, func_name, spec, numparams) \
  struct lisp_object * func_name (struct lisp_object*); \
  void func_name ## _init () { \
    define_builtin_function( lisp_name , spec , numparams , func_name ); \
  } \
  struct lisp_object * func_name (struct lisp_object *args)

C_BOOL true_p(struct lisp_object * obj);

struct lisp_object *make_lisp_object(enum type obj_type, void *data);

#define HEAD(list) ((struct lisp_object *)(list->data))
#define TRUEP(obj) true_p(obj)

/* Possibly changing constants */
/* Not necessarily a max length for a symbol. It is much more efficient if a
 * symbol's length is smaller than this.
 */
#define BASE_SYMBOL_LENGTH 35
/* A scaling factor for when a symbol's length is > BASE_SYMBOL_LENGTH */
#define SYMBOL_SCALE_FACTOR 2

/* Same definitions for string constants */
#define BASE_STRINGBUF_LENGTH 256
#define STRINGBUF_SCALE_FACTOR 2

#define SYMBOL_TABLE_INITIAL_SIZE 100
#define SYMBOL_TABLE_SCALE_FACTOR 2

#define LISPINT int
#define LISPINT_FORMAT "%d"

#define MAX_ERROR 1000

#endif /* LISP_H_ */
