#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eina.h>

#include "eina_suite.h"

#define MAX_ROWS 10
#define MAX_COLS 10

static void eina_matrixsparse_free_cell_cb(void *user_data EINA_UNUSED,
                                           void *cell_data EINA_UNUSED)
{
}

static void matrixsparse_initialize(Eina_Matrixsparse *matrix,
                                    long data[MAX_ROWS][MAX_COLS],
                                    unsigned long nrows,
                                    unsigned long ncols)
{
   unsigned long i, j;
   Eina_Bool r;

   for (i = 0; i < nrows; i++)
      for (j = 0; j < ncols; j++)
         if (data[i][j] != 0)
           {
              r = eina_matrixsparse_data_idx_set(matrix, i, j, &data[i][j]);
                fail_if(r == EINA_FALSE);
           }

}

static void matrixsparse_check(Eina_Matrixsparse *matrix,
                               long data[MAX_ROWS][MAX_COLS],
                               unsigned long nrows EINA_UNUSED,
                               unsigned long ncols EINA_UNUSED)
{
   unsigned long i, j;
   long *test1;

   for (i = 0; i < MAX_ROWS; i++)
      for (j = 0; j < MAX_COLS; j++)
        {
           if (data[i][j] != 0)
             {
                test1 = eina_matrixsparse_data_idx_get(matrix, i, j);
                fail_if(test1 == NULL || *test1 != data[i][j]);
             }
           else
             {
                test1 = eina_matrixsparse_data_idx_get(matrix, i, j);
                fail_if(test1 != NULL);
             }
        }
}

EFL_START_TEST(eina_test_simple)
{
   Eina_Matrixsparse *matrix = NULL;
   Eina_Matrixsparse_Cell *cell = NULL;
   Eina_Bool r;
   long *test1, value, value2, value3, value4;
   unsigned long i, j;
   unsigned long row, col;

   long data[MAX_ROWS][MAX_COLS];

   for (i = 0; i < MAX_ROWS; i++)
      for (j = 0; j < MAX_COLS; j++)
         data[i][j] = 0;

   data[0][3] = 3;
   data[1][3] = 13;
   data[1][6] = 16;
   data[1][9] = 19;
   data[1][8] = 18;
   data[1][7] = 17;
   data[2][8] = 28;
   data[2][7] = 27;
   data[2][6] = 26;
   data[3][5] = 35;
   data[3][6] = 36;
   data[3][7] = 37;
   data[3][9] = 39;
   data[3][0] = 30;
   data[4][6] = 46;
   data[4][8] = 48;
   data[4][2] = 42;
   data[4][3] = 43;
   data[4][7] = 47;
   data[5][3] = 53;
   data[6][3] = 63;
   data[6][4] = 64;
   data[6][6] = 66;
   data[7][3] = 73;
   data[7][7] = 77;
   data[8][8] = 88;

   value = -1;
   value2 = -2;
   value3 = -3;
   value4 = -4;


   matrix = eina_matrixsparse_new(MAX_ROWS, MAX_COLS,
                                  eina_matrixsparse_free_cell_cb, data);
   fail_if(matrix == NULL);

   r = eina_matrixsparse_cell_idx_get(matrix, 3, 5, &cell);
   fail_if(r == EINA_FALSE);
   fail_if(cell != NULL);

   matrixsparse_initialize(matrix, data, MAX_ROWS, MAX_COLS);

   /* data fetching */
   test1 = eina_matrixsparse_data_idx_get(matrix, 3, 0);
   fail_if(test1 == NULL);
   fail_if(*test1 != data[3][0]);

   test1 = eina_matrixsparse_data_idx_get(matrix, 3, 5);
   fail_if(test1 == NULL);
   fail_if(*test1 != data[3][5]);

   test1 = eina_matrixsparse_data_idx_get(matrix, 3, 6);
   fail_if(test1 == NULL);
   fail_if(*test1 != data[3][6]);

   test1 = eina_matrixsparse_data_idx_get(matrix, 3, 1);
   fail_if(test1 != NULL);

   r = eina_matrixsparse_cell_idx_get(matrix, 3, 5, &cell);
   fail_if(r == EINA_FALSE);
   fail_if(cell == NULL);

   test1 = eina_matrixsparse_cell_data_get(cell);
   fail_if(test1 == NULL);
   fail_if(*test1 != data[3][5]);

   r = eina_matrixsparse_cell_position_get(cell, &row, &col);
   fail_if(r == EINA_FALSE);
   fail_if(row != 3 || col != 5);

   test1 = eina_matrixsparse_data_idx_get(matrix, 4, 3);
   fail_if(*test1 != data[4][3]);

   test1 = eina_matrixsparse_data_idx_get(matrix, 1, 3);
   fail_if(*test1 != data[1][3]);

   /* data changing */
   r = eina_matrixsparse_data_idx_set(matrix, 1, 9, &data[1][9]);
   fail_if(r == EINA_FALSE);

   r = eina_matrixsparse_data_idx_replace(matrix, 4, 3, &value, (void **)&test1);
   fail_if(r == EINA_FALSE);
   fail_if(test1 == NULL);
   fail_if(*test1 != data[4][3]);
   data[4][3] = value;

   test1 = eina_matrixsparse_data_idx_get(matrix, 4, 3);
   fail_if(test1 == NULL || *test1 != value);

   r = eina_matrixsparse_cell_data_replace(cell, &value2, (void **)&test1);
   fail_if(r == EINA_FALSE);
   fail_if(test1 == NULL);
   fail_if(*test1 != data[3][5]);
   data[3][5] = value2;

   test1 = eina_matrixsparse_data_idx_get(matrix, 3, 5);
   fail_if(test1 == NULL);
   fail_if(*test1 != value2);

   r = eina_matrixsparse_cell_idx_get(matrix, 4, 2, &cell);
   fail_if(r == EINA_FALSE || cell == NULL);

   r = eina_matrixsparse_cell_data_set(cell, &value3);
   fail_if(r == EINA_FALSE);
   data[4][2] = value3;

   test1 = eina_matrixsparse_data_idx_get(matrix, 4, 2);
   fail_if(test1 == NULL || *test1 != value3);

   r = eina_matrixsparse_data_idx_replace(matrix,
                                          6,
                                          5,
                                          &value4,
                                          (void **)&test1);
   fail_if(r == EINA_FALSE || test1 != NULL);
   data[6][5] = value4;


   /* cell deletion */
   r = eina_matrixsparse_row_idx_clear(matrix, 4);
   fail_if(r == EINA_FALSE);
   data[4][6] = 0;
   data[4][8] = 0;
   data[4][2] = 0;
   data[4][3] = 0;
   data[4][7] = 0;

   test1 = eina_matrixsparse_data_idx_get(matrix, 4, 3);
   fail_if(test1 != NULL);

   test1 = eina_matrixsparse_data_idx_get(matrix, 4, 8);
   fail_if(test1 != NULL);

   test1 = eina_matrixsparse_data_idx_get(matrix, 5, 3);
   fail_if(*test1 != data[5][3]);

   r = eina_matrixsparse_column_idx_clear(matrix, 3);
   fail_if(r != EINA_TRUE);
   data[0][3] = 0;
   data[1][3] = 0;
   data[4][3] = 0;
   data[5][3] = 0;
   data[6][3] = 0;
   data[7][3] = 0;

   r = eina_matrixsparse_cell_idx_clear(matrix, 3, 5);
   fail_if(r != EINA_TRUE);
   data[3][5] = 0;

   r = eina_matrixsparse_cell_idx_clear(matrix, 3, 9);
   fail_if(r != EINA_TRUE);
   data[3][9] = 0;

   r = eina_matrixsparse_cell_idx_clear(matrix, 4, 3);
   fail_if(r != EINA_TRUE);
   data[4][3] = 0;

   r = eina_matrixsparse_cell_idx_get(matrix, 3, 7, &cell);
   fail_if(r == EINA_FALSE);
   fail_if(cell == NULL);

   r = eina_matrixsparse_cell_clear(cell);
   fail_if(r == EINA_FALSE);
   data[3][7] = 0;

   r = eina_matrixsparse_cell_idx_get(matrix, 2, 7, &cell);
   fail_if(r == EINA_FALSE);

   r = eina_matrixsparse_cell_idx_clear(matrix, 2, 8);
   fail_if(r == EINA_FALSE);
   data[2][8] = 0;

   r = eina_matrixsparse_cell_idx_clear(matrix, 2, 7);
   fail_if(r == EINA_FALSE);
   data[2][7] = 0;

   r = eina_matrixsparse_cell_idx_get(matrix, 7, 7, &cell);
   fail_if(r == EINA_FALSE);

   r = eina_matrixsparse_row_idx_clear(matrix, 8);
   fail_if(r == EINA_FALSE);
   data[8][8] = 0;

   r = eina_matrixsparse_row_idx_clear(matrix, 7);
   fail_if(r == EINA_FALSE);
   data[7][3] = 0;
   data[7][7] = 0;

   matrixsparse_check(matrix, data, MAX_ROWS, MAX_COLS);
   eina_matrixsparse_free(matrix);

}
EFL_END_TEST

EFL_START_TEST(eina_test_resize)
{
   Eina_Matrixsparse *matrix = NULL;
   Eina_Bool r;
   unsigned long i, j;
   unsigned long nrows, ncols;

   long data[MAX_ROWS][MAX_COLS];

   for (i = 0; i < MAX_ROWS; i++)
      for (j = 0; j < MAX_COLS; j++)
         data[i][j] = 0;


   matrix = eina_matrixsparse_new(MAX_ROWS, MAX_COLS,
                                  eina_matrixsparse_free_cell_cb, data);
   fail_if(matrix == NULL);

   /* cell insertion */
   data[0][5] = 5;
   data[1][0] = 10;
   data[1][3] = 13;
   data[1][6] = 16;
   data[1][9] = 19;
   data[1][8] = 18;
   data[1][7] = 17;
   data[2][8] = 28;
   data[2][7] = 27;
   data[2][6] = 26;
   data[3][0] = 30;
   data[3][5] = 35;
   data[3][6] = 36;
   data[3][7] = 37;
   data[3][9] = 39;
   data[3][0] = 30;
   data[4][8] = 48;
   data[4][2] = 42;
   data[4][3] = 43;
   data[4][7] = 47;
   data[4][6] = 46;
   data[5][3] = 53;
   data[6][3] = 63;
   data[6][4] = 64;
   data[6][6] = 66;
   data[7][3] = 73;
   data[7][7] = 77;
   data[8][8] = 88;

   matrixsparse_initialize(matrix, data, MAX_ROWS, MAX_COLS);

   eina_matrixsparse_size_get(matrix, &nrows, &ncols);
   fail_if(nrows != MAX_ROWS || ncols != MAX_COLS);

   r = eina_matrixsparse_size_set(matrix, nrows - 2, ncols - 2);
   fail_if(r == EINA_FALSE);
   data[1][9] = 0;
   data[1][8] = 0;
   data[2][8] = 0;
   data[3][9] = 0;
   data[4][8] = 0;
   data[8][8] = 0;
   matrixsparse_check(matrix, data, MAX_ROWS, MAX_COLS);

   r = eina_matrixsparse_size_set(matrix, 5, 1);
   fail_if(r == EINA_FALSE);
   data[0][5] = 0;
   data[1][3] = 0;
   data[1][6] = 0;
   data[1][7] = 0;
   data[2][7] = 0;
   data[2][6] = 0;
   data[3][5] = 0;
   data[3][6] = 0;
   data[3][7] = 0;
   data[4][2] = 0;
   data[4][3] = 0;
   data[4][7] = 0;
   data[4][6] = 0;
   data[5][3] = 0;
   data[6][3] = 0;
   data[6][4] = 0;
   data[6][6] = 0;
   data[7][3] = 0;
   data[7][7] = 0;
   matrixsparse_check(matrix, data, MAX_ROWS, MAX_COLS);

   r = eina_matrixsparse_size_set(matrix, 1, 1);
   fail_if(r == EINA_FALSE);
   data[3][0] = 0;
   data[1][0] = 0;
   matrixsparse_check(matrix, data, MAX_ROWS, MAX_COLS);

   r = eina_matrixsparse_size_set(matrix, 5, 4);
   fail_if(r == EINA_FALSE);

   r = eina_matrixsparse_data_idx_set(matrix, 4, 2, &data[4][2]);
   fail_if(r == EINA_FALSE);
   data[4][2] = 42;
   matrixsparse_check(matrix, data, MAX_ROWS, MAX_COLS);

   r = eina_matrixsparse_size_set(matrix, 5, 1);
   fail_if(r == EINA_FALSE);
   data[4][2] = 0;
   matrixsparse_check(matrix, data, MAX_ROWS, MAX_COLS);

   eina_matrixsparse_free(matrix);

}
EFL_END_TEST

EFL_START_TEST(eina_test_iterators)
{
   Eina_Matrixsparse *matrix = NULL;
   Eina_Matrixsparse_Cell *cell = NULL;
   Eina_Iterator *it = NULL;
   Eina_Bool r;
   long *test1, value;
   unsigned long i, j;
   unsigned long row, col;

   long data[MAX_ROWS][MAX_COLS];

   value = 0;
   for (i = 0; i < MAX_ROWS; i++)
     {
        for (j = 0; j < MAX_COLS; j++)
          {
             data[i][j] = value++;
             printf("%4ld ", data[i][j]);
          }
             printf("\n");
     }


   matrix = eina_matrixsparse_new(MAX_ROWS, MAX_COLS,
                                  eina_matrixsparse_free_cell_cb, data);
   fail_if(matrix == NULL);

   r = eina_matrixsparse_data_idx_set(matrix, 3, 5, &data[3][5]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 3, 6, &data[3][6]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 3, 7, &data[3][7]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 3, 9, &data[3][9]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 3, 0, &data[3][0]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 4, 6, &data[4][6]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 4, 8, &data[4][8]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 4, 2, &data[4][2]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 4, 3, &data[4][3]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 4, 7, &data[4][7]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 6, 4, &data[6][4]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 5, 3, &data[5][3]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 6, 3, &data[6][3]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 7, 3, &data[7][3]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 0, 3, &data[0][3]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 1, 3, &data[1][3]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 1, 6, &data[1][6]);
   fail_if(r == EINA_FALSE);
   r = eina_matrixsparse_data_idx_set(matrix, 1, 9, &data[1][9]);
   fail_if(r == EINA_FALSE);

   it = eina_matrixsparse_iterator_new(matrix);
   fail_if(it == NULL);
   EINA_ITERATOR_FOREACH(it, cell)
   {
      fail_if(cell == NULL);
      r = eina_matrixsparse_cell_position_get(cell, &row, &col);
      fail_if(r == EINA_FALSE);

      test1 = eina_matrixsparse_cell_data_get(cell);
      fail_if(test1 == NULL || *test1 != data[row][col]);
   }
      eina_iterator_free(it);

   it = eina_matrixsparse_iterator_complete_new(matrix);
   fail_if(it == NULL);
   EINA_ITERATOR_FOREACH(it, cell)
   {
         fail_if(cell == NULL);
      r = eina_matrixsparse_cell_position_get(cell, &row, &col);
         fail_if(r == EINA_FALSE);

      test1 = eina_matrixsparse_cell_data_get(cell);
      if (test1)
         fail_if(*test1 != data[row][col]);
   }
         eina_iterator_free(it);

   eina_matrixsparse_free(matrix);

}
EFL_END_TEST

void
eina_test_matrixsparse(TCase *tc)
{
   tcase_add_test(tc, eina_test_simple);
   tcase_add_test(tc, eina_test_resize);
   tcase_add_test(tc, eina_test_iterators);
}
