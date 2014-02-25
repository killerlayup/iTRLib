#include "cameraextercalc.h"
#include "alglib.h"
#include "stdio.h"
#define resid_MAX_2 64
namespace itr_vision
{
CameraExterCalc::CameraExterCalc()
{
    //ctor
    H.Init(3,3);
    R.Init(3,6);
    t.Init(3,2);
    N.Init(3,2);
    V.Init(3,3);
}

CameraExterCalc::~CameraExterCalc()
{
    //dtor
}

CameraExterCalc::CameraExterCalc(const CameraExterCalc &other)
{
    //copy ctor
    this->H=other.H;
    this->V=other.V;
    this->R=other.R;
    this->t=other.t;
    this->N=other.N;
}

BOOL CameraExterCalc::CalcH(VectorFeaturePoint *PointList1,S32 List1Num,VectorFeaturePoint *PointList2,S32 List2Num, S32 matched_num)
{
    Calculate CalculateObj;
    Numerical NumericalObj;
    //筛选最优 H 时所用
    Matrix best_H(3,3);

    Vector pos_2s(3),pos_1(3);
    F32 red_counter,tmp_red,best_counter=0, best_red=0;
    F32 risde;

    //bucketing
    S32 bucket_counter[16]= {0};
    S32 *bucket;
    bucket=new S32 [16*matched_num]();
    //bucket={0};

    F32 *tempvalue_u, *tempvalue_v;
    S32 *tempID;
    tempvalue_u=new F32[matched_num]();
    tempvalue_v=new F32[matched_num]();
    tempID=new S32[matched_num]();
    matched_num=0;
    for(S32 i=0; i<List1Num; i++)
    {
        if(PointList1[i].ID!=-1)
        {
            tempID[matched_num]=i;
            tempvalue_u[matched_num]=PointList1[i].X;
            tempvalue_v[matched_num]=PointList1[i].Y;
            matched_num++;
        }
    }
    F32 U,V;
    CalculateObj.Max(tempvalue_u,matched_num,U);
    CalculateObj.Max(tempvalue_v,matched_num,V);
    /// bucket for 16 bucket
    for(S32 i=0; i<matched_num; i++)
    {
        if(tempvalue_u[i]<=U/4&&tempvalue_v[i]<=V/4)
        {
            bucket_counter[0]++;
            *(bucket+(0*matched_num+bucket_counter[0]))=i;
            continue;
        }
        else if(tempvalue_u[i]<=U/4&&tempvalue_v[i]<=V/2)
        {
            bucket_counter[1]++;
            *(bucket+(1*matched_num+bucket_counter[1]))=i;
            continue;
        }
        else if(tempvalue_u[i]<=U/4&&tempvalue_v[i]<=3*V/4)
        {
            bucket_counter[2]++;
            *(bucket+(2*matched_num+bucket_counter[2]))=i;
            continue;
        }
        else if(tempvalue_u[i]<=U/4)
        {
            bucket_counter[3]++;
            *(bucket+(3*matched_num+bucket_counter[3]))=i;
            continue;
        }
        else if(tempvalue_u[i]<=U/2&&tempvalue_v[i]<=V/4)
        {
            bucket_counter[4]++;
            *(bucket+(4*matched_num+bucket_counter[4]))=i;
            continue;
        }
        else if(tempvalue_u[i]<=U/2&&tempvalue_v[i]<=V/2)
        {
            bucket_counter[5]++;
            *(bucket+(5*matched_num+bucket_counter[5]))=i;
            continue;
        }
        else if(tempvalue_u[i]<=U/2&&tempvalue_v[i]<=3*V/4)
        {
            bucket_counter[6]++;
            *(bucket+(6*matched_num+bucket_counter[6]))=i;
            continue;
        }
        else if(tempvalue_u[i]<=U/2)
        {
            bucket_counter[7]++;
            *(bucket+(7*matched_num+bucket_counter[7]))=i;
            continue;
        }
        else if(tempvalue_u[i]<=3*U/4&&tempvalue_v[i]<=V/4)
        {
            bucket_counter[8]++;
            *(bucket+(8*matched_num+bucket_counter[8]))=i;
            continue;
        }
        else if(tempvalue_u[i]<=3*U/4&&tempvalue_v[i]<=V/2)
        {
            bucket_counter[9]++;
            *(bucket+(9*matched_num+bucket_counter[9]))=i;
            continue;
        }
        else if(tempvalue_u[i]<=3*U/4&&tempvalue_v[i]<=3*V/4)
        {
            bucket_counter[10]++;
            *(bucket+(10*matched_num+bucket_counter[10]))=i;
            continue;
        }
        else if(tempvalue_u[i]<=3*U/4)
        {
            bucket_counter[11]++;
            *(bucket+(11*matched_num+bucket_counter[11]))=i;
            continue;
        }
        else if(tempvalue_v[i]<=V/4)
        {
            bucket_counter[12]++;
            *(bucket+(12*matched_num+bucket_counter[12]))=i;
            continue;
        }
        else if(tempvalue_v[i]<=V/2)
        {
            bucket_counter[13]++;
            *(bucket+(13*matched_num+bucket_counter[13]))=i;
            continue;
        }
        else if(tempvalue_v[i]<=3*V/4)
        {
            bucket_counter[14]++;
            *(bucket+(14*matched_num+bucket_counter[14]))=i;
            continue;
        }
        else
        {
            bucket_counter[15]++;
            *(bucket+(15*matched_num+bucket_counter[15]))=i;
            continue;
        }
    }
    S32 amptybucket=0;
    for(S32 i=0; i<16; i++)
        if(bucket_counter[i]==0)
            amptybucket++;
    assert(amptybucket<=12);

    F32 ratio_bucket[16]= {0};
    ratio_bucket[0]=((F32)bucket_counter[0])/matched_num;
    for(S32 i=1; i<16; i++)
    {
        ratio_bucket[i]=ratio_bucket[i-1] + (F32)bucket_counter[i]/matched_num;
    }
    /// RANSAC,calculate H
    S16 b[4]= {0};
    S32 c[4]= {0};
    F32 p,wght,u1,v1,u2,v2;
    S32 q,tmp_k;
    S32 i=0;
    do      ///for(S32 i=0; i<20; i++)     /// RANSAC begin:
    {
        //pick 4 buckets
        for(S32 j=0; j<4; j++)
        {
            NumericalObj.Rand(p);
            for(S32 k=0; k<16; k++)
            {
                if(k==0)
                {
                    if(p<ratio_bucket[0])
                    {
                        q=1;
                        break;
                    }
                }
                else
                {
                    if(ratio_bucket[k-1]<p&&ratio_bucket[k]>=p)
                    {
                        if(bucket_counter[k]>0)
                            q=k;
                        else
                        {
                            tmp_k =k;
                            while(bucket_counter[tmp_k]==0)
                                tmp_k--;
                            q=tmp_k;
                        }
                        break;
                    }
                }
            }
            if(j>0)
            {
                for(S32 z=0; z<j; z++)
                    if(q==b[z])
                    {
                        q=-1;
                        j--;
                    }
            }
            if(q!=-1)
                b[j]=q;
        }
        //pick 4 feature point
        for(S32 j=0; j<4; j++)
        {
            NumericalObj.Rand(p);
            NumericalObj.Ceil(p*bucket_counter[b[j]],q);
            c[j]=tempID[*(bucket+(b[j]*matched_num+q))];
        }
        //matrix
        double M[72]={0};  //最小二乘矩阵
        for(S32 j=0; j<4; j++)
        {
            u1 = PointList1[c[j]].X;
            v1 = PointList1[c[j]].Y;
            u2 = PointList2[PointList1[c[j]].ID].X;
            v2 = PointList2[PointList1[c[j]].ID].Y;
            wght = PointList2[PointList1[c[j]].ID].Quality;
            M[2*j*9+3]=-u1*wght;
            M[2*j*9+4]=-v1*wght;
            M[2*j*9+5]=-wght;
            M[2*j*9+6]=u1*v2*wght;
            M[2*j*9+7]=v1*v2*wght;
            M[2*j*9+8]=v2*wght;
            M[(2*j+1)*9+0]=-u1*wght;
            M[(2*j+1)*9+1]=-v1*wght;
            M[(2*j+1)*9+2]=-wght;
            M[(2*j+1)*9+6]=u1*u2*wght;
            M[(2*j+1)*9+7]=v1*u2*wght;
            M[(2*j+1)*9+8]=u2*wght;
        }
        //svd
        double V[81]={0};
        alglib::real_2d_array Marray,Varray,Uarray;
        alglib::real_1d_array Warray;
        Marray.setcontent(8,9,M);
        Varray.setcontent(9,9,V);
        alglib::rmatrixsvd(Marray,8,9,0,2,2,Warray,Uarray,Varray);

        for(S32 j=0; j<3; j++)
            for(S32 k=0; k<3; k++)
                H(j,k)=Varray(8,j*3+k);

        //残差量
        red_counter=0,tmp_red=0;
        pos_1[2]=1;
        for(S32 j=0; j<matched_num; j++)
        {
            pos_1[0]=tempvalue_u[j];
            pos_1[1]=tempvalue_v[j];

            pos_2s=H*pos_1;

            pos_2s[0]/=pos_2s[2];
            pos_2s[1]/=pos_2s[2];

            pos_2s[0]-=PointList2[PointList1[tempID[j]].ID].X;
            pos_2s[1]-=PointList2[PointList1[tempID[j]].ID].Y;
            CalculateObj.MultiSum(pos_2s.GetData(),pos_2s.GetData(),2,risde);

            if(tempID[j]==c[0]||tempID[j]==c[1]||tempID[j]==c[2]||tempID[j]==c[3])
                risde=0;
            if(risde<resid_MAX_2)   /// 与王论文相比 risde 未开方
                red_counter++;
            tmp_red+=risde;
        }

        if(i==0)
        {
            best_H=H;
            best_counter=red_counter;
            best_red=tmp_red;
        }
        else
        {
            if((red_counter>best_counter)||((red_counter==best_red)&&(tmp_red<best_red)))
            {
                best_red = tmp_red;
                best_counter = red_counter;
                best_H = H;
            }
        }
        i++;
    }while(best_counter<0.33*matched_num&&i<10);//);//end of RANSAC &&i<40

    H=best_H;

    delete[] bucket;
    delete[] tempvalue_u;
    delete[] tempvalue_v;
    delete[] tempID;

    return true;
}
/**
* \brief 通过给定的相机内参数和深度参数D计算运动参数(R,T,N,V)
* \param CameraInterPara 相机内参数
* \param D 深度(单位:米)
* \note 此计算步骤需已完成单应性矩阵计算(既已成功调用CalcHV())
*/
//double*matrixmulti(Matrix &)
BOOL CameraExterCalc::CalcMotion(CameraInterCalc &CameraInterPara,F32 D)
{
    Matrix tmpA(3,3);
    Matrix tmpApinv(3,3);
    Matrix Hc(3,3);
    CameraInterPara.MatC2P.CopyTo(0,0,3,3,tmpA.GetData());
    tmpA.Inv(tmpApinv);
/// //////////////////////////////////////////////////////////////
//printf("tmpAinv:\n");                            /// //////////////////
//        for(S32 j=0; j<3; j++)              /// //////////////////
//        {   for(S32 k=0; k<3; k++)          /// //////////////////
//                printf("%f\t",tmpApinv(j,k));    /// //////////////////
//            printf("\n");                   /// //////////////////
//        }                                   /// //////////////////
/// //////////////////////////////////////////////////////////////

    Hc=tmpApinv*H*tmpA;
/// //////////////////////////////////////////////////////////////
//printf("H(homo):\n");                            /// //////////////////
//        for(S32 j=0; j<3; j++)              /// //////////////////
//        {   for(S32 k=0; k<3; k++)          /// //////////////////
//                printf("%f\t",H(j,k));     /// //////////////////
//            printf("\n");                   /// //////////////////
//        }                                   /// //////////////////
//printf("Hc(H):\n");                            /// //////////////////
//        for(S32 j=0; j<3; j++)              /// //////////////////
//        {   for(S32 k=0; k<3; k++)          /// //////////////////
//                printf("%f\t",Hc(j,k));     /// //////////////////
//            printf("\n");                   /// //////////////////
//        }                                   /// //////////////////
/// //////////////////////////////////////////////////////////////
    //Matrix V(3,3),U(3,3);
    //Vector d(3);
    double Udata[9]={0},Vdata[9]={0},Ddata[3]={0},Hdata[9];
    for(S32 i=0; i<3; i++)
        for(S32 j=0; j<3; j++)
            Hdata[i*3+j]=(double)H(i,j);///          ///////////////////////////////////////

    alglib::real_2d_array Harray,Varray,Uarray;
    alglib::real_1d_array Darray;
    Harray.setcontent(3,3,Hdata);
    Varray.setcontent(3,3,Vdata);
    Uarray.setcontent(3,3,Udata);
    Darray.setcontent(3,Ddata);
    alglib::rmatrixsvd(Harray,3,3,2,2,2,Darray,Uarray,Varray);

/// //////////////////////////////////////////////////////////////
//printf("U:\n");                             /// //////////////////
//        for(S32 j=0; j<3; j++)              /// //////////////////
//        {   for(S32 k=0; k<3; k++)          /// //////////////////
//                printf("%f\t",Uarray(j,k));      /// //////////////////
//            printf("\n");                   /// //////////////////
//        }                                   /// //////////////////
//
//printf("d:\n");                             /// //////////////////
//        for(S32 j=0; j<3; j++)              /// //////////////////
//            printf("%f\t",Darray[j]);            /// //////////////////
//        printf("\n");                       /// //////////////////
//
//printf("Vt:\n");                            /// //////////////////
//        for(S32 j=0; j<3; j++)              /// //////////////////
//        {   for(S32 k=0; k<3; k++)          /// //////////////////
//                printf("%f\t",Varray(j,k));     /// //////////////////
//            printf("\n");                   /// //////////////////
//        }                                   /// //////////////////
/// //////////////////////////////////////////////////////////////
    Matrix U(3,3);
    for(S32 j=0; j<3; j++)
        for(S32 k=0; k<3; k++)
            U(j,k)=(F32)Uarray(j,k);
    for(S32 j=0; j<3; j++)
        for(S32 k=0; k<3; k++)
            V(j,k)=(F32)Varray(j,k);
    Vector u1v(3),u3v(3),v1v(3),v3v(3);
    F32 q1,q2,q3;
    U.CopyColTo(0, u1v.GetData());
    U.CopyColTo(2, u3v.GetData());
    q1=(F32)Darray[0],q2=(F32)Darray[1],q3=(F32)Darray[2];
    V.CopyRowTo(0, v1v.GetData());
    V.CopyRowTo(2, v3v.GetData());

    F32 err1=q1-q2;
    F32 err2=q2-q3;
    F32 err_ratio=0.005*q2;
    S16 state;
    if (err1<=err_ratio && err2<=err_ratio)
        state=1;
    else
        if(err1>err_ratio&& err2>err_ratio)
            state=3;
        else
            state=2;

    F32 s,stmp;
    U.Det(s);
    V.Det(stmp);
    s*=stmp;

    Matrix R1m(3,3);
    Matrix temp33(3,3),u3m(3,1),v3m(3,1),v3tm(1,3);
    U.CopyColTo(2, u3m.GetData());
    V.CopyRowTo(2, v3tm.GetData());
    V.CopyRowTo(2, v3m.GetData());
    Vector t1v(3);
    switch (state)
    {
        case 1:
        {
            R1m.CopyFrom(0,0,3,3,H.GetData());
            R1m.AllMul(1/q2);
            t1v.Set(0);
            R.CopyFrom(0,0,3,3,R1m.GetData());
            t.Set(0);
        }break;
        case 2:
        {
            temp33=u3m*v3tm;
            temp33.AllMul(q3/q1-s);
            R1m.CopyFrom(0,0,3,3,H.GetData());
            R1m.AllMul(1/q2);
            R1m=R1m-temp33; ///R1 = H/q1 - (q3/q1-s)*u3*v3'

            t1v=u3v;
            t1v.Mul(D*(q3/q1-s));///

            double ndata[3]={Varray(2,0),Varray(2,1),Varray(2,2)},nddata[3]={0};
            alglib::real_2d_array narray,nvarray,nuarray;
            alglib::real_1d_array ndarray;
            narray.setcontent(3,1,ndata);
            ndarray.setcontent(3,nddata);
            alglib::rmatrixsvd(narray,3,1,0,0,2,ndarray,nuarray,nvarray);
            F32 W_n=(F32)ndarray[0]; /// W_n=normest(n),n=v3
            t1v.Mul(-W_n);
            R.CopyFrom(0,0,3,3,R1m.GetData());
            t.CopyFrom(0,0,1,3,t1v.GetData());
            N(0,0)=V(2,0),N(1,0)=V(2,1),N(2,0)=V(2,2);/// N=V(2,:);
        }break;
        case 3:
        {
            F32 r=sqrt((q1*q1-q2*q2)/(q2*q2-q3*q3));
            F32 a=(q1+s*q3*r*r)/(q2+q2*r*r);
            F32 b=-sqrt(1-a*a);
            temp33(0,0)=a;  temp33(0,1)=0;  temp33(0,2)=b;
            temp33(1,0)=0;  temp33(1,1)=1;  temp33(1,2)=0;
            temp33(2,0)=-s*b;  temp33(2,1)=0;  temp33(2,2)=s*a;

            R1m=U*temp33*V;
//printf("r:%f\na:%f\nb:%f\n",r,a,b);
//printf("R1:\n");                            /// //////////////////
//        for(S32 j=0; j<3; j++)              /// //////////////////
//        {   for(S32 k=0; k<3; k++)          /// //////////////////
//                printf("%f\t",R1m(j,k));     /// //////////////////
//            printf("\n");                   /// //////////////////
//        }                                   /// //////////////////

            Vector n1v(3);
            n1v=v1v;
            n1v.Mul(r);
            n1v=n1v+v3v;

            Vector temp3v(3);
            t1v=u1v; t1v.Mul(-b);
            temp3v=u3v; temp3v.Mul(q3/q2-s*a);
            t1v=t1v+temp3v;
            t1v.Mul(D);  //t1=(u1*(-b)+u3*(q3/q2-s*a))*D;

            N.CopyFrom(0,0,1,3,n1v.GetData());
            /// normest(n1v)
            double ndata[3]={(double)n1v[0],(double)n1v[1],(double)n1v[2]},nddata[3]={0};
            alglib::real_2d_array narray,nvarray,nuarray;
            alglib::real_1d_array ndarray;
            narray.setcontent(3,1,ndata);
            ndarray.setcontent(3,nddata);
            alglib::rmatrixsvd(narray,3,1,0,0,2,ndarray,nuarray,nvarray);

            F32 W_n=ndarray[0];
//printf("W_n:%f\n",W_n);
            t1v.Mul(-W_n);
            R.CopyFrom(0,0,3,3,R1m.GetData());
            t.CopyFrom(0,0,1,3,t1v.GetData());


            r=-r;
            b=-b;
            temp33(0,2)=b;
            temp33(2,0)=-s*b;
            Matrix R2m(3,3);
            R2m=U*temp33*V;
//printf("R2:\n");                            /// //////////////////
//        for(S32 j=0; j<3; j++)              /// //////////////////
//        {   for(S32 k=0; k<3; k++)          /// //////////////////
//                printf("%f\t",R2m(j,k));     /// //////////////////
//            printf("\n");                   /// //////////////////
//        }
//printf("temp33:\n");                            /// //////////////////
//        for(S32 j=0; j<3; j++)              /// //////////////////
//        {   for(S32 k=0; k<3; k++)          /// //////////////////
//                printf("%f\t",temp33(j,k));     /// //////////////////
//            printf("\n");                   /// //////////////////
//        }
            Matrix n2m(3,1);
            for(S32 j=0; j<3; j++)//n2m = Vrow1
                n2m(j,0)=(F32)Varray(0,j);
//printf("v1:\n");
// for(S32 j=0; j<3; j++)              /// //////////////////
//        {            /// //////////////////
//                printf("%f\t",n2m(j,0));     /// //////////////////
//                              /// //////////////////
//        }
//printf("\nr: %f\n",r);
//printf("v3:\n");
// for(S32 j=0; j<3; j++)              /// //////////////////
//        {            /// //////////////////
//                printf("%f\t",v3m(j,0));     /// //////////////////
//                              /// //////////////////
//        }
            n2m.AllMul(r);
            n2m=n2m+v3m;

            Vector t2v(3);
            t2v=u1v; t2v.Mul(-b);

            temp3v=u3v; temp3v.Mul(q3/q2-s*a);
            t2v=t2v+temp3v; t2v.Mul(D);
            //t2=(u1*(-b)+u3*(q3/q2-s*a))*D;
            //
            N.CopyFrom(0,1,1,3,n2m.GetData());
            for(S32 j=0; j<3; j++)
                narray(j,0)=(double)n2m(j,0);
            alglib::rmatrixsvd(narray,3,1,0,0,2,ndarray,nuarray,nvarray);
            W_n=ndarray[0];
            t2v.Mul(W_n);
            t2v.Mul(-1);
            R.CopyFrom(0,3,3,3,R2m.GetData());
            t.CopyFrom(0,1,1,3,t2v.GetData());

        }break;
    }
    V=V.Tran();
    return true;
}

}
