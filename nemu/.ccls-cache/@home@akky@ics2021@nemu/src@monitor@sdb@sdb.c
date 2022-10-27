#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include "memory/vaddr.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets()
{
  static char *line_read = NULL;

  if (line_read)
  {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read)
  {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args)
{
  cpu_exec(-1);
  return 0;
}

static int cmd_si(char *args)
{
  cpu_exec(*(uint *)args);
  return 0;
}

static int cmd_info(char *args)
{
  if (strcmp(args, "r") == 0)
  {
    isa_reg_display();
  }
  else if (strcmp(args, "w") == 0)
  {
    // do display info
  }
  else
  {
    return -1;
  }
  return 0;
}

static int cmd_memory(char *args)
{
  char *limit = NULL, *exprstr = NULL;
  char *token = NULL, *tmp = NULL;
  int ind;
  for (tmp = args, ind = 0;; tmp = NULL, ind++)
  {
    token = strtok(tmp, " ");
    if (token == NULL)
    {
      break;
    }
    if (ind > 1)
    {
      nemu_state.state = NEMU_ABORT;
      return -1;
    }

    if (ind == 0)
    {
      limit = token;
    }
    if (ind == 1)
    {
      exprstr = token;
    }
  }
  int len = atoi(limit);
  bool b_expr;
  //uint64_t loc = strtoull(exprstr, NULL, 16);
  uint64_t loc = expr(exprstr, &b_expr);
  if (!b_expr)
  {
    nemu_state.state = NEMU_ABORT;
    return -1;
  }
  
  for (size_t i = 0; i < len; i++)
  {
    word_t v = vaddr_read(loc + 4 * i, 4);
    printf("loc:%#08lx, memval:%#08lx\n", loc + i * 4, v);
  }

  printf("\n");
  return 0;
}

static int cmd_q(char *args)
{
  nemu_state.state = NEMU_END;
  return -1;
}

static int cmd_help(char *args);

static struct
{
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"si", "Step by args", cmd_si},
    {"q", "Exit NEMU", cmd_q},
    {"info", "display program info", cmd_info},
    {"x", "display program memory", cmd_memory}

    /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args)
{
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL)
  {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++)
    {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else
  {
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(arg, cmd_table[i].name) == 0)
      {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode()
{
  is_batch_mode = true;
}

void sdb_mainloop()
{
  if (is_batch_mode)
  {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;)
  {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL)
    {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end)
    {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(cmd, cmd_table[i].name) == 0)
      {
        if (cmd_table[i].handler(args) < 0)
        {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD)
    {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb()
{
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
