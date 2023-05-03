/*
 * Name:        dict.c
 * Description: Offline dictionary.
 * Author:      cosh.cage#hotmail.com
 * File ID:     0123230200Z0503231750L00219
 * License:     Public domain.
 */
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "svset.h"
#include "svtree.h"

typedef struct st_WORD
{
	ptrdiff_t id;
	char name[64];
	size_t times;
	long ltip;
} WORD, * P_WORD;

int cbfcmpid(const void * px, const void * py)
{
	return (*(P_WORD)px).id - (*(P_WORD)py).id;
}

int cbfcmpchar(const void * px, const void * py)
{
	return *(char *)px - *(char *)py;
}

int cbftvs_history(void * pitem, size_t param)
{
	if (((P_WORD)P2P_TNODE_B(pitem)->pdata)->times)
		printf("%s\t%lld\n", ((P_WORD)P2P_TNODE_B(pitem)->pdata)->name, ((P_WORD)P2P_TNODE_B(pitem)->pdata)->times);
	return CBF_CONTINUE;
}

int cbftvs_alphabet(void * pitem, size_t param)
{
	if (toupper(((P_WORD)P2P_TNODE_B(pitem)->pdata)->name[0]) == param)
		printf("%s\n", ((P_WORD)P2P_TNODE_B(pitem)->pdata)->name);
	return CBF_CONTINUE;
}

int cbftvs_rwords(void * pitem, size_t param)
{
	if (--*(size_t *)param > 0)
		if (rand() & 1)
			printf("%s\n", ((P_WORD)P2P_TNODE_B(pitem)->pdata)->name);
	return CBF_CONTINUE;
}

void GenerateARandomWord(P_BSTNODE pnode, size_t n)
{
	if (NULL == pnode)
		return;
	while (--n)
	{
		P_BSTNODE t = P2P_BSTNODE(pnode->knot.ppnode[rand() & 1]);
		if (NULL == t)
			break;
		pnode = t;
	}
	printf("%s\n", ((P_WORD)(pnode->knot.pdata))->name);
}

static char sWord[BUFSIZ * 2] = { 0 };
static char * p = sWord;

static char sFileName[BUFSIZ] = { 0 };
static char sPattern[BUFSIZ] = { 0 };

int main(int argc, char ** argv)
{
	size_t i = 1, j;
	WORD w = { 0 };
	FILE * fp;
	P_SET_T set = setCreateT();
	P_TRIE_A trie = treCreateTrieA();
	size_t * result = NULL;

	strcat(sFileName, "./dict.txt");
	fp = fopen(sFileName, "rb");


	printf("Dict file: %s\n", sFileName);
	if (NULL != fp)
	{
		while (!feof(fp))
		{
			*p = fgetc(fp);
			++p;
			if ('#' == *(p - 1))
			{
				P_BSTNODE pnode = NULL;
				*(p - 1) = '\0';
				p = sWord;

				strcpy(w.name, p);
				w.id = i;
				w.times = 0;
				w.ltip = ftell(fp) + 1;

				setInsertT(set, &w, sizeof w, cbfcmpid);
				pnode = treBSTFindData_A(*set, &i, cbfcmpid);
				treInsertTrieA(trie, p, strlen(p), sizeof(char), (size_t)(pnode->knot.pdata), cbfcmpchar);

				++i;
			}
			if ('\n' == *(p - 1))
			{
				p = sWord;
			}
		}
		printf("%lld words loaded.\n", i);
		do
		{
			printf("? ");
			fgets(sPattern, 100, stdin);
			sPattern[strlen(sPattern) - 1] = '\0';
			if ('\0' == *sPattern)
				break;
			printf("Searching: \"%s\"...\n", sPattern);
			result = treSearchTrieA(trie, sPattern, strlen(sPattern), sizeof(char), cbfcmpchar);
			if (result)
			{
				printf("\t%lld %s  ", ((P_WORD)*result)->id, ((P_WORD)*result)->name);
				++((P_WORD)*result)->times;

				/* Redirect to the word on the disk. */
				fseek(fp, ((P_WORD)*result)->ltip, SEEK_SET);
				/* Read explanations. */
				p = sWord;
				while ('\n' != (*p = fgetc(fp)))
				{
					++p;
				}
				*p = '\0';
				p = sWord;
				printf("\t%s\n", p);
			}
			else if ('.' == sPattern[0] && '?' == sPattern[1])
			{
				printf("Type [WORD] or [NUMBER] to search.\n");
				printf("\tFor example ? Apple ? 10536\n");
				printf("Type .h to show history.\n");
				printf("Type .l[A] to show alphabet.\n");
				printf("\tFor example ? .l Z.\n");
				printf("Type .g to generate random words.\n");
				printf("Type .? to show this notice.\n");
			}
			else if ('.' == sPattern[0] && 'h' == sPattern[1])
			{
				printf("History:\n");
				treTraverseBYIn(*set, cbftvs_history, 0);
			}
			else if ('.' == sPattern[0] && 'l' == sPattern[1] && ' ' == sPattern[2])
			{
				sPattern[3] = toupper(sPattern[3]);
				printf("Alphabet:\n");
				treTraverseBYIn(*set, cbftvs_alphabet, toupper(sPattern[3]));
			}
			else if ('.' == sPattern[0] && 'g' == sPattern[1])
			{
				size_t n, m, x;
				srand(time(NULL));
				printf("How many random words would you like to gen:");
				(void)scanf("%llu", &n);
				printf("\n");
				/* Calculate the height of a tree. */
				i = (size_t)(logf(i + 1) / logf(2));
				for (m = 0; m < n; ++m)
				{
					x = 1 + rand() / ((RAND_MAX + 1u) / i);
					GenerateARandomWord(*set, x);
				}
			}
			else if (0 != (j = atoi(sPattern)))
			{
				P_BSTNODE pnode = treBSTFindData_A(*set, &j, cbfcmpid);
				if (pnode)
				{
					printf("\t%lld %s  ", ((P_WORD)(pnode->knot.pdata))->id, ((P_WORD)(pnode->knot.pdata))->name);
					++((P_WORD)(pnode->knot.pdata))->times;

					/* Redirect to the word on the disk. */
					fseek(fp, ((P_WORD)(pnode->knot.pdata))->ltip, SEEK_SET);
					/* Read explanations. */
					p = sWord;
					while ('\n' != (*p = fgetc(fp)))
					{
						++p;
					}
					*p = '\0';
					p = sWord;
					printf("\t%s\n", p);
				}
			}
			else
			{
				printf("Can not find \"%s\".\n", sPattern);
			}
		} while ('\0' != sPattern[0]);
		fclose(fp);
	}
	else
		printf("Can not open file.\n");

	printf("History:\n");
	treTraverseBYIn(*set, cbftvs_history, 0);
	setDeleteT(set);
	treDeleteTrieA(trie, sizeof(char));
	return 0;
}
