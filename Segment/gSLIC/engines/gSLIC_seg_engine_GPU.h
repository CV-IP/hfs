#pragma once
#include "gSLIC_seg_engine.h"

namespace gSLIC
{
	namespace engines
	{
		class seg_engine_GPU : public seg_engine
		{
		private:

			int no_grid_per_center;
			SpixelMap* accum_map;
			IntImage* tmp_idx_img;

		protected:
			void Cvt_Img_Space(UChar4Image* inimg, Float4Image* outimg, COLOR_SPACE color_space);
			void Init_Cluster_Centers();
			void Find_Center_Association();
			void Update_Cluster_Center();
			void Enforce_Connectivity();

		public:

			seg_engine_GPU(const objects::settings& in_settings);
			~seg_engine_GPU();

			void Draw_Segmentation_Result(UChar4Image* out_img);
		};
	}
}

