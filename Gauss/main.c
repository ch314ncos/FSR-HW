#include <stdio.h> 
#include <math.h>


int main(){
    int n;
    scanf("%d", &n);
    double det=1;
    double a[n][n];
    double chst;

    // ввод матрицы
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            scanf("%lf", &a[i][j]);
        }
    }

    double iline , kline, c;
    for (int i=0; i<n-1; i++){
        for (int k=i+1; k<n; k++){
            // смена строк при нулях на гл диаг
            if (a[i][i]==0 && a[k][i]!=0){
                for (int j=0; j<n; j++){
                    c = a[i][j];
                    a[i][j] = a[k][j];
                    a[k][j] = c;
                }
                det*=-1;
            }
        
			// приведение к верхнетреугольному виду
            iline= a[i][i];
            kline = a[k][i];
            if (kline!=0){
                for (int j=0; j<n; j++){
                    a[k][j] = a[k][j] * iline   - kline*a[i][j];
                }
                det/=iline;
               }
        }
    }

    for (int i=0; i<n; i++){
        det *= a[i][i];
    }

    fabs(det)<1e-9 ? printf("0\n") : printf("%.0lf\n", det);
    

    return 0;
}
