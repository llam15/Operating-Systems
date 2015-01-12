#include "alloc.h"
#include "command-internals.h"
#include "command.h"
#include "parser.h"
#include <stdint.h>

static Token_t* g_tok_list;
static uint64_t g_tok_index;
static uint64_t g_tok_list_len;
static command_t root;
static command_t cur_node;

static uint64_t line_num;

void parse(Token_t* tok_list, uint64_t tok_list_len)
{
  root = NULL;
  cur_node = root;
  g_tok_index = 0;
  g_tok_list = tok_list;
  g_tok_list_len = tok_list_len;
  shell();
}

bool getTok(void)
{
  if (g_tok_index++ == g_tok_list_len)
    return true;
  return false;
}

//Note: Make sure that parent and child are both malloc'd
void insert_child(command_t parent, command_t child)
{
  uint8_t num_children = 0;
  switch(parent->type) {
  case IF_COMMAND:
    num_children = 3;
    break;
  case PIPE_COMMAND:
    num_children = 2;
    break;
  case SEQUENCE_COMMAND:
    num_children = 2;
    break;
  case SUBSHELL_COMMAND:
    num_children = 1;
    break;
  case UNTIL_COMMAND:
    num_children = 2;
    break;
  case WHILE_COMMAND:
    num_children = 2;
    break;
  }

  if (num_children == 0)
    fprintf(stderr, "%llu: Syntax Error: Token nesting not supported for command num %d\n", line_num, parent->type);

  uint8_t child_index;
  for(child_index = 0; child_index < num_children; child_index++)
  {
    if(parent->command[child_index] == NULL)
    {
      parent->command[child_index] = child;
      return;
    }
  }

  fprintf(stderr, "%llu: Syntax Error: Too many child tokens following command num %d\n", line_num, parent->type);
}

// Note: returns the newly created node
command_t insert_node(command_type type)
{
  
    command_t node = (command_t) checked_malloc(sizeof(command));
    node->type = type;
    if (root == NULL)
      root = node;
    
    if(cur_node != NULL)
    {
      insert_child(cur_node, node);
    }

    cur_node = node;
    return node;
}

void shell()
{
  line_num = g_tok_list[g_tok_index].line_num;
  switch (g_tok_list[g_tok_index].type) {
  case TOK_IF:
    insert_node(IF_COMMAND);
    break;
  case TOK_PIPE:
    insert_node(PIPE_COMMAND);
    break;
  case TOK_SC:
    insert_node(SEQUENCE_COMMAND);
    break;
  case TOK_WORD:
    insert_node(SIMPLE_COMMAND);
    break;
  case TOK_LPAREN:
    insert_node(SUBSHELL_COMMAND);
    break;
  case TOK_UNTIL:
    insert_node(UNTIL_COMMAND);
    break;
  case TOK_WHILE:
    insert_node(WHILE_COMMAND);
    break;  
  }

}
