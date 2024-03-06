#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>

extern "C" {

#define CRASH char *p = NULL;free(p);
#define cost(i, j) (m[dim*(i)+j])
#define v(i) (v[i])
#define u(i) (u[i])
#define x(i) (x[i])
#define rowsol(i) (rowsol[i])
#define colsol(i) (colsol[i])
#define matches(i) (matches[i])
#define freerows(i) (freerows[i])
#define collist(i) (collist[i])
#define x(i) (x[i])
#define pred(i) (pred[i])
#define minimum(x) (*std::min_element(x, x + dim))
#define min_index(x) ((int)(std::min_element(x, x + dim) - x))
#define dim 12

int m[dim * dim];
int v[dim];
int u[dim];
int rowsol[dim];
int colsol[dim];
int matches[dim];
int freerows[dim];
int numfree;

void print_m(int *colsol = NULL) {
    char buf[BUFSIZ];
    for (int k = 0; k < 3; k++) {
        printf("   ");
        for (int j = 0; j < dim; j++) {
            snprintf(buf, BUFSIZ, "%3d", j);
            int ii = 0;
            while (char c = buf[ii++]) {
                printf("%c\u{fe09}", c);
            }
        }
        if (k < 2) {
            printf("       ");
        } else {
            printf("\n");
        }
    }
    for (int i = 0; i < dim; i++) {
        for (int k = 0; k < 3; k++) {
            snprintf(buf, BUFSIZ, "%3d", i);
            int ii = 0;
            while (char c = buf[ii++]) {
                printf("%c\u{fe09}", c);
            }
            for (int j = 0; j < dim; j++) {
                int cost = cost(i, j);
                if (k == 1) {
                    cost -= v[j];
                } else if (k == 2) {
                    cost -= u[i] + v[j];
                }
                if (colsol && colsol[j] == i) {
                    snprintf(buf, BUFSIZ, "%3d", cost);
                    int ii = 0;
                    while (char c = buf[ii++]) {
                        printf("%c\u{fe06}", c);
                    }
                } else {
                    printf("%3d", cost);
                }
            }
            printf("%3d", u[i]);
            if (k < 2) {
                printf("    ");
            } else {
                printf("\n");
            }
        }
    }
    for (int k = 0; k < 3; k++) {
        printf("   ");
        for (int j = 0; j < dim; j++) {
            printf("%3d", v[j]);
        }
        if (k < 2) {
            printf("       ");
        } else {
            printf("\n");
        }
    }
    int min = 0;
    for (int i = 0; i < dim; i++) {
        min += u[i] + v[i];
    }
    printf("minimum: %d\n\n", min);
}

void random_m() {
    srand((unsigned int)time(NULL));
    int max = std::min(dim * dim, 100);
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            cost(i, j) = rand() % (max - 1) + 1;
        }
    }
    printf("int mm[] = {");
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            printf("%d, ", cost(i, j));
        }
    }
    printf("};memcpy(m, mm, sizeof(m));\n");
}

void col_reduction() {
    printf("column reduction\n");
    for (int j = 0; j < dim; j++) {
        // find minimum cost over rows.
        int dmin = cost(0,j);
        int imin = 0;
        for (int i = 1; i < dim; i++) {
            if (cost(i,j) < dmin) {
                dmin = cost(i,j);
                imin = i;
            }
        }
        v(j) = dmin;
        matches(imin)++;
        if (matches(imin) == 1) {
            // init assignment if minimum row assigned for first time.
            rowsol(imin) = j;
            colsol(j) = imin;
        } else if (v(j) < v(rowsol(imin))) {
            int jone = rowsol(imin);
            rowsol(imin) = j;
            colsol(j) = imin;
            colsol(jone) = -1;
        } else {
            colsol(j) = -1; // row already assigned, column not assigned.
        }
    }
    print_m(colsol);
}

void reduction_transfer() {
    printf("reduction transfer (finding subminimums in rows with only one minimum)\n");
    for (int i = 0; i < dim; i++) {
        switch (matches(i)) {
            case 0:
                freerows(numfree++) = i;
                break;
            case 1:
                printf("checking row %d\n", i);
                int x[dim];
                for (int j = 0; j < dim; j++) {
                    x(j) = cost(i,j) - v(j);
                }
                int jone = rowsol(i);
                x(jone) = INT_MAX;
                u(i) = minimum(x);
                printf("subminimum: %d\n", u(i));
                v(jone) = v(jone) - u(i);
                print_m(colsol);
        }
    }
}

void augmenting_row_reduction() {
    printf("augmenting row reduction (finding subminimums in free rows)\n");
    int loopcnt = 0; // do-loop to be done twice.
    while (loopcnt < 1) {
        printf("loop %d, numfree: %d\n", loopcnt, numfree);
        loopcnt++;
        //     scan all free rows.
        //     in some cases, a free row may be replaced with another one to be scanned next.
        int k = 0;
        int prvnumfree = numfree;
        numfree = 0;             // start list of rows still free after augmenting row reduction.
        while (k < prvnumfree) {
            int i = freerows(k);
            printf("checking row %d (numfree %d)\n", i, numfree);
            k++;
            // find minimum and second minimum reduced cost over columns.
            int x[dim];
            for (int j = 0; j < dim; j++) {
                x(j) = cost(i,j) - v(j);
            }
            
            int jone = min_index(x);
            int umin = x(jone);
            x(jone) = INT_MAX;
            
            int jtwo = min_index(x);
            int usubmin = x(jtwo);
            printf("umin: %d (column: %d), usubmin: %d (column: %d)\n", umin, jone, usubmin, jtwo);
            int i0 = colsol(jone);
            u[i] = usubmin;
            if (usubmin > umin) {
                // change the reduction of the minimum column to increase the minimum
                // reduced cost in the row to the subminimum.
                v(jone) -= usubmin - umin;
            } else { // minimum and subminimum equal.
                if (i0 > -1) { // minimum column jone is assigned.
                    // swap columns jone and jtwo, as jtwo may be unassigned.
                    jone = jtwo;
                    i0 = colsol(jtwo);
                }
            }
            // (re-)assign i to jone, possibly de-assigning an i0.
            rowsol(i) = jone;
            colsol(jone) = i;
            if (i0 > -1) {  // minimum column jone assigned earlier. // ORIGINAL
                if (usubmin > umin) {
                    // put in current k, and go back to that k.
                    // continue augmenting path i - jone with i0.
                    k--;
                    freerows(k) = i0;
                } else {
                    // no further augmenting reduction possible.
                    // store i0 in list of free rows for next phase.
                    freerows(numfree++) = i0;
                }
            }
            print_m(colsol);
        }
    }
}

void augment() {
    printf("augment free rows, numfree: %d\n", numfree);
    for (int f = 0; f < numfree; f++) {
        int freerow = freerows(f);
        printf("checking free row %d\n", freerow);
        int x[dim];
        int pred[dim];
        int collist[dim];
        for (int j = 0; j < dim; j++) {
            x(j) = cost(freerow,j) - v(j);
            pred(j) = freerow;
            collist(j) = j; // init column list.
        }
        int low = 0;    // columns in 0..low-1 are ready, now none.
        int up = 0;     // columns in low..up-1 are to be scanned for current minimum, now none.
        // columns in up..dim-1 are to be considered later to find new minimum,
        // at this stage the list simply contains all columns
        int last = 0;
        int xmin = 0;
        int endofpath = 0;
        bool unassignedfound = false;
        while (!unassignedfound) {
            if (up == low) { // no more columns to be scanned for current minimum.
                last = low - 1;
                // scan columns for up..dim-1 to find all indices for which new minimum occurs.
                // store these indices between low..up-1 (increasing up).
                xmin = x(collist(up));
                up++;
                for (int k = up; k < dim; k++) {
                    int j = collist(k);
                    int h = x(j);
                    if (h <= xmin) { //
                        if (h < xmin) {  // new minimum.
                            up = low;   // restart list at index low.
                            xmin = h;
                        }
                        // new index with same minimum, put on undex up, and extend list.
                        collist(k) = collist(up);
                        collist(up) = j;
                        up++;
                    }
                }
                // check if any of the minimum columns happens to be unassigned.
                // if so, we have an augmenting path right away.
                for (int k = low; k < up; k++) {
                    printf("found minimum %d at column %d\n", xmin, collist(k));
                    if (colsol(collist(k)) < 0) {
                        printf("found unassigned column %d\n", collist(k));
                        endofpath = collist(k);
                        unassignedfound = true;
                        break;
                    }
                }
            }
            if (!unassignedfound) {
                // update 'distances' between freerow and all unscanned columns, via next scanned column.
                int jone = collist(low);
                low++;
                int i = colsol(jone);
                printf("checking row %d for new minimum\n", i);
                int h = cost(i,jone) - v(jone) - xmin;
                if (cost(i,jone) - cost(freerow, jone) != h) { CRASH }
                for (int k = up; k < dim; k++) {
                    int j = collist(k);
                    int v2 = cost(i,j) - v(j) - h;
                    if (v2 < x(j)) {
                        printf("found new minimum at column %d (%d - %d (= %d) < %d)\n", j, cost(i,j) - v(j), h, v2, x(j));
                        pred(j) = i;
                        if (v2 == xmin) {  // new column found at same minimum value - ORIGINAL
                            printf("minimum has same minimum value (%d == %d)\n", v2, xmin);
                            if (colsol(j) < 0) {
                                // if unassigned, shortest augmenting path is complete.
                                printf("found unassigned column %d\n", j);
                                endofpath = j;
                                unassignedfound = true;
                                break;
                            } else {
                                // else add to list to be scanned right away.
                                collist(k) = collist(up);
                                collist(up) = j;
                                up++;
                            }
                        }
                        x(j) = v2;
                    }
                }
            }
        }
        
        // update column prices.
        for (int k = 0; k <= last; k++) {
            int j1 = collist(k);
            v(j1) += x(j1) - xmin;
        }
        
        // reset row and column assignments along the alternating path.
        while (true) {
            int i = pred(endofpath);
            colsol(endofpath) = i;
            int j = endofpath;
            endofpath = rowsol(i);
            rowsol(i) = j;
            if (i == freerow) {
                break;
            }
        }
        print_m(colsol);
    }
}

void brute(int i, int *l, int *min, int sum = 0) {
    if (i == dim) {
        memcpy(rowsol, l, sizeof(rowsol));
        *min = sum;
        return;
    }
    for (int j = 0; j < dim; j++) {
        if (sum + cost(i,j) >= *min) {
            continue;
        }
        bool found = false;
        for (int k = 0; k < dim; k++) {
            if (l[k] == j) {
                found = true;
                break;
            }
        }
        if (found) {
            continue;
        }
        sum += cost(i,j);
        l[i] = j;
        brute(i + 1, l, min, sum);
        l[i] = -1;
        sum -= cost(i, j);
    }
}

void brute_force() {
    printf("brute force\n");
    int l[dim];
    int k = -1;
    memset_pattern4(l, &k, sizeof(l));
    int min = INT_MAX;
    brute(0, l, &min);
    for (int i = 0; i < dim; i++) {
        colsol[rowsol[i]] = i;
    }
    memset(u, 0, sizeof(u));
    memset(v, 0, sizeof(v));
    print_m(colsol);
    printf("brute force minimum: %d\n", min);
}

int main(int argc, const char * argv[]) {
//        random_m();
//    int mm[] = {9, 24, 19, 10, 23, 19, 13, 21, 10, 15, 16, 2, 4, 20, 9, 24, 5, 22, 16, 7, 3, 9, 17, 24, 13, };memcpy(m, mm, sizeof(m));
//    int mm[] = {6, 9, 13, 16, 16, 1, 24, 7, 11, 11, 4, 13, 11, 6, 21, 2, 8, 10, 4, 11, 9, 8, 24, 8, 1, };memcpy(m, mm, sizeof(m));
    int mm[] = { 21,16,13,26,17,20,2,22,13,6,2,21,23,8,7,2,17,1,11,11,11,10,5,27,14,6,14,10,9,0,3,2,16,25,13,2,3,15,15,0,18,10,9,17,27,22,21,14,10,8,14,2,9,17,22,15,0,11,27,24,15,22,27,27,15,24,26,5,4,18,25,14,22,4,8,6,16,17,25,18,1,21,18,16,13,13,20,1,24,19,22,17,11,9,15,12,19,6,12,2,17,8,27,18,22,23,3,24,12,23,26,4,13,14,17,22,2,26,13,21,25,6,11,22,12,0,12,2,21,9,16,2,12,23,1,2,16,15,27,24,22,22,7,25,};memcpy(m, mm, sizeof(m));
    print_m();
    col_reduction();
    reduction_transfer();
    augmenting_row_reduction();
    augment();
    brute_force();
    return 0;
}

}
