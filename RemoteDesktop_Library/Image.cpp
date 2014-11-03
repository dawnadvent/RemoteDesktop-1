#include "stdafx.h"
#include "Image.h"

RemoteDesktop::Rect RemoteDesktop::Image::Difference(Image first, Image second, int horz_jump ){
	int top = -1;
	int bottom = -1;
	int left = -1;
	int right = -1;


	auto even = (first.width % horz_jump);
	auto totalwidth = first.width - even; //subtract any extra bits to ensure I dont go over the array bounds
	auto stide = (first.width * 4);
	for (int y = 0; y < first.height; y++)
	{
		auto linea = (int*)(first.data + (stide *y));
		auto lineb = (int*)(second.data + (stide *y));

		for (int x = 0; x < totalwidth; x += horz_jump)
		{

			auto la = linea[x] + linea[x + 1];
			auto lb = lineb[x] + lineb[x + 1];
			if (la != lb)
			{
				auto tmpx = x;
				if (top == -1)
				{
					top = y;
					if (bottom == -1)
						bottom = y + 1;
				}
				else
					bottom = y + 1;
				if (left == -1)
				{
					left = tmpx;
					if (right == -1)
						right = tmpx + 1;
				}
				else if (tmpx < left)
				{
					left = tmpx;
				}
				else if (right < tmpx)
				{
					right = tmpx + 1;
				}

			}

		}

	}


	if ((right - left <= 0) || (bottom - top <= 0))
		return Rect();
	if (left > 0)
		left -= 1;
	if (top > 0)
		top -= 1;
	if (top > 0)
		top -= 1;
	if (top > 0)
		top -= 1;
	if (right < second.width - 1)
		right += 1;
	if (right < second.width - 1)
		right += 1;
	if (right < second.width - 1)
		right += 1;
	if (bottom < second.height - 1)
		bottom += 1;

	return Rect(top, left, right - left, bottom - top);


}

RemoteDesktop::Image RemoteDesktop::Image::Copy(Image src_img, Rect src_copy_region, std::vector<unsigned char>& buffer)
{
	auto size = src_copy_region.width * src_copy_region.height * 4;
	buffer.reserve(size);
	auto src = src_img.data;
	auto dst = buffer.data();

	auto srcrowstride = src_img.width * 4;
	auto dstrowstride = src_copy_region.width * 4;
	for (int y = 0; y <src_copy_region.height; y++)
	{
		auto srcrow = (int*)(src + (srcrowstride * (y + src_copy_region.top)));
		srcrow += src_copy_region.left;
		auto dstrow = dst + (dstrowstride * y);
		memcpy(dstrow, srcrow, dstrowstride);
	}
	return Image(buffer.data(), size, src_copy_region.height, src_copy_region.width, false);
}
void RemoteDesktop::Image::Copy(Image src_img, int dst_left, int dst_top, int dst_stride, unsigned char* dst)
{
	auto src = src_img.data;
	auto srcrowstride = src_img.width * 4;
	for (int y = 0; y <src_img.height; y++)
	{
		auto dstrow = (int*)(dst + (dst_stride * (y + dst_top)));
		dstrow += dst_left;
		auto srcrow = src + (srcrowstride * y);
		memcpy(dstrow, srcrow, srcrowstride);
	}
}