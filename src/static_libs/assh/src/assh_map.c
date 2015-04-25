/*

  libassh - asynchronous ssh2 client/server library.

  Copyright (C) 2013 Alexandre Becoulet <alexandre.becoulet@free.fr>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301 USA

*/

#include <assh/assh.h>
#include <assh/assh_map.h>

#define ASSH_MAP_W (sizeof(assh_map_id_t) * 8)

struct assh_map_entry_s *
assh_map_insert(struct assh_map_entry_s **root,
		struct assh_map_entry_s *item)
{
  while (1)
    {
      struct assh_map_entry_s *e = *root;
      if (e == NULL)
        {
          item->lz = ASSH_MAP_W;
          item->link[0] = item->link[1] = 0;
          *root = item;
          return NULL;
        }
      assh_map_id_t a = item->id, b = e->id, x = a ^ b;
      if (x & (-2LL << (ASSH_MAP_W - 1 - e->lz)))
        {
          item->lz = ASSH_CLZ32(x);
          item->link[0] = 0;
          item->link[1] = e;
          *root = item;
          return NULL;
        }
      if (a == b)
        return e;
      root = &e->link[(x >> (ASSH_MAP_W - 1 - e->lz)) & 1];
    }
}

void
assh_map_remove(struct assh_map_entry_s **parent,
                struct assh_map_entry_s *item)
{
  if (!item->link[0])
    {
      *parent = item->link[1];
      return;
    }

  struct assh_map_entry_s **p = &item->link[0], *e = *p;
  while (e->link[0])
    {
      p = &e->link[0];
      e = *p;
    }
  *p = e->link[1];
  *parent = e;
  e->lz = item->lz;
  e->link[0] = item->link[0];
  e->link[1] = item->link[1];
}

struct assh_map_entry_s *
assh_map_lookup(struct assh_map_entry_s **root,
		assh_map_id_t id, struct assh_map_entry_s ***parent)
{
  struct assh_map_entry_s *r = *root;

  while (r != NULL)
    {
      assh_map_id_t x = id ^ r->id;
      if (!x)
        break;
      if ((x & (-2LL << (ASSH_MAP_W - 1 - r->lz))))
        return NULL;
      root = r->link + ((x >> (ASSH_MAP_W - 1 - r->lz)) & 1);
      r = *root;
    }
  if (parent != NULL)
    *parent = root;
  return r;
}

struct assh_map_entry_s *
assh_map_head(struct assh_map_entry_s **root,
	      struct assh_map_entry_s ***parent)
{
  struct assh_map_entry_s *r = *root;

  while (r != NULL)
    {
      root = r->link + 0;
      r = *root;
    }
  if (parent != NULL)
    *parent = root;
  return r;
}

void assh_map_iter(struct assh_map_entry_s *root, void *ctx,
		   void (*iter)(struct assh_map_entry_s *, void *))
{
  struct assh_map_entry_s *stack[ASSH_MAP_W], *next, *l1;
  int i = 0;

  while (1)
    {
      for (; root != NULL; root = next)
        {
          next = root->link[0];
          l1 = root->link[1];
          if (l1 != NULL)
            stack[i++] = l1;
          iter(root, ctx);
        }
      if (i == 0)
        break;
      root = stack[--i];
    }
}

