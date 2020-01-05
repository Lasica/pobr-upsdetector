/*
 * Author: Artur Dobrogowski
 * 2020-01-04
 */

#include "segmentfiller.h"
SegmentFiller::SegmentFiller(cv::Mat &vm, Segment &s) : segment(s) {
    mask_ptr = vm.ptr<unsigned char>(0);
    n_rows = vm.rows;
    n_cols = vm.cols;
}

    
void SegmentFiller::operator()(Coord x, Coord y) {
    // Process the pixel as segment fragment
    segment.addPoint(x, y, *(mask_ptr + n_cols*x + y));
    *(mask_ptr + n_cols*x + y) = 0;
    for(int i=0; i < 4; ++i) {
        const Coord new_x = x_translation[i] + x;
        const Coord new_y = y_translation[i] + y;
        
        // Validate coordinates
        if(new_x > 0 && new_x < n_rows && new_y > 0 && new_y < n_cols)
            
            // If mask not yet processed
            if (*(mask_ptr + n_cols * new_x + new_y) > 0)
                
                // Process pixel of new coordinates as segment fragment
                this->operator()(new_x, new_y);
    }
}
