#ifndef CONNECTEDANALYSIS_H
#define CONNECTEDANALYSIS_H

#include "itrbase.h"
#include <vector>
#include "../feature/block.h"
#include <math.h>
using std::vector;
using itr_math::Matrix;
namespace itr_vision
{
    class ConnectedAnalysis
    {
        public:
            /**
            * \brief 构造函数
            */
            ConnectedAnalysis();
            ~ConnectedAnalysis();
            /**
            * \brief 对图像做连通域分析
            * \param input 输入图像
            * \param blocks 所有连通域的信息，按面积从大到小排序
            */
            void Contour(const Matrix &input,vector<Block> &blocks);

            //const static int dx[8] = {-1,0,1,-1,1,-1,0,1};
            //const static int dy[8] = {-1,-1,-1,0,0,1,1,1};
            int dx[8];
            int dy[8];
            S32 ImgWidth;
            S32 ImgHeight;
            S32 BNum;
        protected:
        private:
            /**
            * \brief 种子染色
            * \param input 输入图像
            * \param x,y 横纵坐标
            * \param blk 当前被染色的区域
            * \param visited 被访问标识
            */
            void Fill(const Matrix &input,S32 x,S32 y,Block &blk,F32* visited);
            /**
            * \brief 联通域排序（按面积降序排列）
            * \param blocks 联通域向量
            */
            void SortBlocks(vector<Block> &blocks);
            /**
            * \brief 判断像素相等
            * \param Val1,Val2 像素值
            */
            inline bool PixEql(F32 Val1,F32 Val2)
            {
                if(fabs(Val1-Val2)<0.1)
                    return true;
                else
                    return false;
            }
    };
}
#endif // CONNECTEDANALYSIS_H
