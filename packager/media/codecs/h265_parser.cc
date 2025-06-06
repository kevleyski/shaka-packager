// Copyright 2016 Google LLC. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include <packager/media/codecs/h265_parser.h>

#include <algorithm>
#include <cmath>

#include <absl/log/check.h>
#include <absl/log/log.h>

#include <packager/macros/compiler.h>
#include <packager/macros/logging.h>
#include <packager/media/codecs/nalu_reader.h>

#define TRUE_OR_RETURN(a)                            \
  do {                                               \
    if (!(a)) {                                      \
      DVLOG(1) << "Failure while processing " << #a; \
      return kInvalidStream;                         \
    }                                                \
  } while (0)

#define OK_OR_RETURN(a)  \
  do {                   \
    Result status = (a); \
    if (status != kOk)   \
      return status;     \
  } while (false)

#define READ_LONG_OR_RETURN(out)                                           \
  do {                                                                     \
    int _top_half, _bottom_half;                                           \
    if (!br->ReadBits(16, &_top_half)) {                                   \
      DVLOG(1)                                                             \
          << "Error in stream: unexpected EOS while trying to read " #out; \
      return kInvalidStream;                                               \
    }                                                                      \
    if (!br->ReadBits(16, &_bottom_half)) {                                \
      DVLOG(1)                                                             \
          << "Error in stream: unexpected EOS while trying to read " #out; \
      return kInvalidStream;                                               \
    }                                                                      \
    *(out) = ((long)_top_half) << 16 | _bottom_half;                       \
  } while (false)

namespace shaka {
namespace media {

namespace {
int GetNumPicTotalCurr(const H265SliceHeader& slice_header,
                       const H265Sps& sps,
                       const H265Vps& vps,
                       int nuh_layer_id,
                       int nuh_temporal_id) {
  int num_pic_total_curr = 0;
  const H265ReferencePictureSet& ref_pic_set =
      slice_header.short_term_ref_pic_set_sps_flag
          ? sps.st_ref_pic_sets[slice_header.short_term_ref_pic_set_idx]
          : slice_header.st_ref_pic_set;

  for (int i = 0; i < ref_pic_set.num_negative_pics; i++) {
    if (ref_pic_set.used_by_curr_pic_s0[i])
      num_pic_total_curr++;
  }
  for (int i = 0; i < ref_pic_set.num_positive_pics; i++) {
    if (ref_pic_set.used_by_curr_pic_s1[i])
      num_pic_total_curr++;
  }

  num_pic_total_curr += slice_header.used_by_curr_pic_lt;

  int num_active_ref_layer_pics = 0;
  if (nuh_layer_id > 0) {
    int num_ref_layer_pics = 0;
    for (int i = 0; i < vps.num_direct_ref_layers[nuh_layer_id]; i++) {
      int ref_layer_idx =
          vps.layer_id_in_vps[vps.id_direct_ref_layers[nuh_layer_id][i]];
      if (vps.sub_layers_vps_max_minus1[ref_layer_idx] >= nuh_temporal_id &&
          (nuh_temporal_id == 0 ||
           vps.max_tid_il_ref_pics_plus1[ref_layer_idx]
                                        [vps.layer_id_in_vps[nuh_layer_id]] >
               nuh_temporal_id)) {
        num_ref_layer_pics++;
      }
    }
    if (num_ref_layer_pics > 0) {
      if (vps.default_ref_layers_active_flag) {
        num_active_ref_layer_pics = num_ref_layer_pics;
      } else if (!slice_header.inter_layer_pred_enabled_flag) {
        num_active_ref_layer_pics = 0;
      } else if (vps.max_one_active_ref_layer_flag ||
                 vps.num_direct_ref_layers[nuh_layer_id] == 1) {
        num_active_ref_layer_pics = 1;
      } else {
        LOG(ERROR)
            << "For stereo views, num_direct_ref_layers is expected to be 1.";
      }
    }
  }
  return num_pic_total_curr + num_active_ref_layer_pics;
}

void GetAspectRatioInfo(const H265Sps& sps,
                        uint32_t* pixel_width,
                        uint32_t* pixel_height) {
  // The default value is 0; so if this is not in the SPS, it will correctly
  // assume unspecified.
  int aspect_ratio_idc = sps.vui_parameters.aspect_ratio_idc;

  // Table E.1
  switch (aspect_ratio_idc) {
    case 1:  *pixel_width = 1;   *pixel_height = 1;  break;
    case 2:  *pixel_width = 12;  *pixel_height = 11; break;
    case 3:  *pixel_width = 10;  *pixel_height = 11; break;
    case 4:  *pixel_width = 16;  *pixel_height = 11; break;
    case 5:  *pixel_width = 40;  *pixel_height = 33; break;
    case 6:  *pixel_width = 24;  *pixel_height = 11; break;
    case 7:  *pixel_width = 20;  *pixel_height = 11; break;
    case 8:  *pixel_width = 32;  *pixel_height = 11; break;
    case 9:  *pixel_width = 80;  *pixel_height = 33; break;
    case 10: *pixel_width = 18;  *pixel_height = 11; break;
    case 11: *pixel_width = 15;  *pixel_height = 11; break;
    case 12: *pixel_width = 64;  *pixel_height = 33; break;
    case 13: *pixel_width = 160; *pixel_height = 99; break;
    case 14: *pixel_width = 4;   *pixel_height = 3;  break;
    case 15: *pixel_width = 3;   *pixel_height = 2;  break;
    case 16: *pixel_width = 2;   *pixel_height = 1;  break;

    case 255:
      *pixel_width = sps.vui_parameters.sar_width;
      *pixel_height = sps.vui_parameters.sar_height;
      break;

    default:
      // Section E.3.1 specifies that other values should be interpreted as 0.
      LOG(WARNING) << "Unknown aspect_ratio_idc " << aspect_ratio_idc;
      FALLTHROUGH_INTENDED;
    case 0:
      // Unlike the spec, assume 1:1 if not specified.
      *pixel_width = 1;
      *pixel_height = 1;
      break;
  }
}
}  // namespace

bool ExtractResolutionFromSps(const H265Sps& sps,
                              uint32_t* coded_width,
                              uint32_t* coded_height,
                              uint32_t* pixel_width,
                              uint32_t* pixel_height) {
  int crop_x = 0;
  int crop_y = 0;
  if (sps.conformance_window_flag) {
    int sub_width_c = 0;
    int sub_height_c = 0;

    // Table 6-1
    switch (sps.chroma_format_idc) {
      case 0:  // Monochrome
        sub_width_c = 1;
        sub_height_c = 1;
        break;
      case 1:  // 4:2:0
        sub_width_c = 2;
        sub_height_c = 2;
        break;
      case 2:  // 4:2:2
        sub_width_c = 2;
        sub_height_c = 1;
        break;
      case 3:  // 4:4:4
        sub_width_c = 1;
        sub_height_c = 1;
        break;
      default:
        LOG(ERROR) << "Unexpected chroma_format_idc " << sps.chroma_format_idc;
        return false;
    }

    // Formula D-28, D-29
    crop_x =
        sub_width_c * (sps.conf_win_right_offset + sps.conf_win_left_offset);
    crop_y =
        sub_height_c * (sps.conf_win_bottom_offset + sps.conf_win_top_offset);
  }

  // Formula D-28, D-29
  *coded_width = sps.pic_width_in_luma_samples - crop_x;
  *coded_height = sps.pic_height_in_luma_samples - crop_y;
  GetAspectRatioInfo(sps, pixel_width, pixel_height);
  return true;
}

H265Pps::H265Pps() {}
H265Pps::~H265Pps() {}

H265Sps::H265Sps() {}
H265Sps::~H265Sps() {}

H265Vps::H265Vps() {}
H265Vps::~H265Vps() {}

int H265Sps::GetPicSizeInCtbsY() const {
  int min_cb_log2_size_y = log2_min_luma_coding_block_size_minus3 + 3;
  int ctb_log2_size_y =
      min_cb_log2_size_y + log2_diff_max_min_luma_coding_block_size;
  int ctb_size_y = 1 << ctb_log2_size_y;

  // Round-up division.
  int pic_width_in_ctbs_y = (pic_width_in_luma_samples - 1) / ctb_size_y + 1;
  int pic_height_in_ctbs_y = (pic_height_in_luma_samples - 1) / ctb_size_y + 1;
  return pic_width_in_ctbs_y * pic_height_in_ctbs_y;
}

int H265Sps::GetChromaArrayType() const {
  if (!separate_colour_plane_flag)
    return chroma_format_idc;
  else
    return 0;
}

H265ReferencePictureListModifications::H265ReferencePictureListModifications() {
}
H265ReferencePictureListModifications::
    ~H265ReferencePictureListModifications() {}

H265SliceHeader::H265SliceHeader() {}
H265SliceHeader::~H265SliceHeader() {}

H265Parser::H265Parser() {}
H265Parser::~H265Parser() {}

H265Parser::Result H265Parser::ParseSliceHeader(const Nalu& nalu,
                                                H265SliceHeader* slice_header) {
  DCHECK(nalu.is_video_slice());
  *slice_header = H265SliceHeader();

  // Parses whole element.
  H26xBitReader reader;
  reader.Initialize(nalu.data() + nalu.header_size(), nalu.payload_size());
  H26xBitReader* br = &reader;

  TRUE_OR_RETURN(br->ReadBool(&slice_header->first_slice_segment_in_pic_flag));
  if (nalu.type() >= Nalu::H265_BLA_W_LP &&
      nalu.type() <= Nalu::H265_RSV_IRAP_VCL23) {
    TRUE_OR_RETURN(br->ReadBool(&slice_header->no_output_of_prior_pics_flag));
  }

  TRUE_OR_RETURN(br->ReadUE(&slice_header->pic_parameter_set_id));
  const H265Pps* pps = GetPps(slice_header->pic_parameter_set_id);
  TRUE_OR_RETURN(pps);

  const H265Sps* sps = GetSps(pps->seq_parameter_set_id);
  TRUE_OR_RETURN(sps);

  const H265Vps* vps = GetVps(sps->video_parameter_set_id);
  const int nuh_layer_id = nalu.nuh_layer_id();
  if (nuh_layer_id > 0) {
    TRUE_OR_RETURN(vps);
  }

  if (!slice_header->first_slice_segment_in_pic_flag) {
    if (pps->dependent_slice_segments_enabled_flag) {
      TRUE_OR_RETURN(br->ReadBool(&slice_header->dependent_slice_segment_flag));
    }
    const int bit_length = ceil(log2(sps->GetPicSizeInCtbsY()));
    TRUE_OR_RETURN(br->ReadBits(bit_length, &slice_header->segment_address));
  }

  if (!slice_header->dependent_slice_segment_flag) {
    TRUE_OR_RETURN(br->SkipBits(pps->num_extra_slice_header_bits));
    TRUE_OR_RETURN(br->ReadUE(&slice_header->slice_type));
    if (pps->output_flag_present_flag) {
      TRUE_OR_RETURN(br->ReadBool(&slice_header->pic_output_flag));
    }
    if (sps->separate_colour_plane_flag) {
      TRUE_OR_RETURN(br->ReadBits(2, &slice_header->colour_plane_id));
    }

    if ((nuh_layer_id > 0 &&
         !vps->poc_lsb_not_present_flag[vps->layer_id_in_vps[nuh_layer_id]]) ||
        (nalu.type() != Nalu::H265_IDR_W_RADL &&
         nalu.type() != Nalu::H265_IDR_N_LP)) {
      TRUE_OR_RETURN(br->ReadBits(sps->log2_max_pic_order_cnt_lsb_minus4 + 4,
                                  &slice_header->slice_pic_order_cnt_lsb));
    }

    if (nalu.type() != Nalu::H265_IDR_W_RADL &&
        nalu.type() != Nalu::H265_IDR_N_LP) {
      TRUE_OR_RETURN(
          br->ReadBool(&slice_header->short_term_ref_pic_set_sps_flag));
      if (!slice_header->short_term_ref_pic_set_sps_flag) {
        OK_OR_RETURN(ParseReferencePictureSet(
            sps->num_short_term_ref_pic_sets, sps->num_short_term_ref_pic_sets,
            sps->st_ref_pic_sets, br, &slice_header->st_ref_pic_set));
      } else if (sps->num_short_term_ref_pic_sets > 1) {
        TRUE_OR_RETURN(
            br->ReadBits(ceil(log2(sps->num_short_term_ref_pic_sets)),
                         &slice_header->short_term_ref_pic_set_idx));
        TRUE_OR_RETURN(slice_header->short_term_ref_pic_set_idx <
                       sps->num_short_term_ref_pic_sets);
      }

      if (sps->long_term_ref_pic_present_flag) {
        if (sps->num_long_term_ref_pics > 0) {
          TRUE_OR_RETURN(br->ReadUE(&slice_header->num_long_term_sps));
        }
        TRUE_OR_RETURN(br->ReadUE(&slice_header->num_long_term_pics));

        const int pic_count =
            slice_header->num_long_term_sps + slice_header->num_long_term_pics;
        slice_header->long_term_pics_info.resize(pic_count);
        for (int i = 0; i < pic_count; i++) {
          if (i < slice_header->num_long_term_sps) {
            int lt_idx_sps = 0;
            if (sps->num_long_term_ref_pics > 1) {
              TRUE_OR_RETURN(br->ReadBits(
                  ceil(log2(sps->num_long_term_ref_pics)), &lt_idx_sps));
            }
            if (sps->used_by_curr_pic_lt_flag[lt_idx_sps])
              slice_header->used_by_curr_pic_lt++;
          } else {
            TRUE_OR_RETURN(br->SkipBits(sps->log2_max_pic_order_cnt_lsb_minus4 +
                                        4));  // poc_lsb_lt
            bool used_by_curr_pic_lt_flag;
            TRUE_OR_RETURN(br->ReadBool(&used_by_curr_pic_lt_flag));
            if (used_by_curr_pic_lt_flag)
              slice_header->used_by_curr_pic_lt++;
          }
          TRUE_OR_RETURN(br->ReadBool(&slice_header->long_term_pics_info[i]
                                           .delta_poc_msb_present_flag));
          if (slice_header->long_term_pics_info[i].delta_poc_msb_present_flag) {
            TRUE_OR_RETURN(br->ReadUE(
                &slice_header->long_term_pics_info[i].delta_poc_msb_cycle_lt));
          }
        }
      }

      if (sps->temporal_mvp_enabled_flag) {
        TRUE_OR_RETURN(
            br->ReadBool(&slice_header->slice_temporal_mvp_enabled_flag));
      }
    }

    if (nuh_layer_id > 0 && !vps->default_ref_layers_active_flag &&
        vps->num_direct_ref_layers[nuh_layer_id] > 0) {
      TRUE_OR_RETURN(
          br->ReadBool(&slice_header->inter_layer_pred_enabled_flag));
      if (slice_header->inter_layer_pred_enabled_flag &&
          vps->num_direct_ref_layers[nuh_layer_id] > 1) {
        NOTIMPLEMENTED()
            << "For stereo video, num_direct_ref_layers is expected to be 1.";
        return kUnsupportedStream;
      }
    }

    if (sps->sample_adaptive_offset_enabled_flag) {
      TRUE_OR_RETURN(br->ReadBool(&slice_header->slice_sao_luma_flag));
      if (sps->GetChromaArrayType() != 0) {
        TRUE_OR_RETURN(br->ReadBool(&slice_header->slice_sao_chroma_flag));
      }
    }

    slice_header->num_ref_idx_l0_active_minus1 =
        pps->num_ref_idx_l0_default_active_minus1;
    slice_header->num_ref_idx_l1_active_minus1 =
        pps->num_ref_idx_l1_default_active_minus1;
    if (slice_header->slice_type == kPSlice ||
        slice_header->slice_type == kBSlice) {
      TRUE_OR_RETURN(
          br->ReadBool(&slice_header->num_ref_idx_active_override_flag));
      if (slice_header->num_ref_idx_active_override_flag) {
        TRUE_OR_RETURN(br->ReadUE(&slice_header->num_ref_idx_l0_active_minus1));
        if (slice_header->slice_type == kBSlice) {
          TRUE_OR_RETURN(
              br->ReadUE(&slice_header->num_ref_idx_l1_active_minus1));
        }
      }

      const int num_pic_total_curr = GetNumPicTotalCurr(
          *slice_header, *sps, *vps, nuh_layer_id, nalu.nuh_temporal_id());
      if (pps->lists_modification_present_flag && num_pic_total_curr > 1) {
        OK_OR_RETURN(SkipReferencePictureListModification(
            *slice_header, *pps, num_pic_total_curr, br));
      }

      if (slice_header->slice_type == kBSlice) {
        TRUE_OR_RETURN(br->ReadBool(&slice_header->mvd_l1_zero_flag));
      }
      if (pps->cabac_init_present_flag) {
        TRUE_OR_RETURN(br->ReadBool(&slice_header->cabac_init_flag));
      }
      if (slice_header->slice_temporal_mvp_enabled_flag) {
        if (slice_header->slice_type == kBSlice) {
          TRUE_OR_RETURN(br->ReadBool(&slice_header->collocated_from_l0));
        }
        bool l0_greater_than_0 = slice_header->num_ref_idx_l0_active_minus1 > 0;
        bool l1_greater_than_0 = slice_header->num_ref_idx_l1_active_minus1 > 0;
        if (slice_header->collocated_from_l0 ? l0_greater_than_0
                                             : l1_greater_than_0) {
          TRUE_OR_RETURN(br->ReadUE(&slice_header->collocated_ref_idx));
        }
      }

      if ((pps->weighted_pred_flag && slice_header->slice_type == kPSlice) ||
          (pps->weighted_bipred_flag && slice_header->slice_type == kBSlice)) {
        OK_OR_RETURN(SkipPredictionWeightTable(
            slice_header->slice_type == kBSlice, *sps, *slice_header, br));
      }
      TRUE_OR_RETURN(br->ReadUE(&slice_header->five_minus_max_num_merge_cand));
    }

    TRUE_OR_RETURN(br->ReadSE(&slice_header->slice_qp_delta));
    if (pps->slice_chroma_qp_offsets_present_flag) {
      TRUE_OR_RETURN(br->ReadSE(&slice_header->slice_cb_qp_offset));
      TRUE_OR_RETURN(br->ReadSE(&slice_header->slice_cr_qp_offset));
    }

    if (pps->chroma_qp_offset_list_enabled_flag) {
      TRUE_OR_RETURN(
          br->ReadBool(&slice_header->cu_chroma_qp_offset_enabled_flag));
    }
    if (pps->deblocking_filter_override_enabled_flag) {
      TRUE_OR_RETURN(
          br->ReadBool(&slice_header->deblocking_filter_override_flag));
    }
    if (slice_header->deblocking_filter_override_flag) {
      TRUE_OR_RETURN(
          br->ReadBool(&slice_header->slice_deblocking_filter_disabled_flag));
      if (!slice_header->slice_deblocking_filter_disabled_flag) {
        TRUE_OR_RETURN(br->ReadSE(&slice_header->slice_beta_offset_div2));
        TRUE_OR_RETURN(br->ReadSE(&slice_header->slice_tc_offset_div2));
      }
    }
    if (pps->loop_filter_across_slices_enabled_flag &&
        (slice_header->slice_sao_luma_flag ||
         slice_header->slice_sao_chroma_flag ||
         !slice_header->slice_deblocking_filter_disabled_flag)) {
      TRUE_OR_RETURN(br->ReadBool(
          &slice_header->slice_loop_filter_across_slices_enabled_flag));
    }
  }

  if (pps->tiles_enabled_flag || pps->entropy_coding_sync_enabled_flag) {
    TRUE_OR_RETURN(br->ReadUE(&slice_header->num_entry_point_offsets));
    if (slice_header->num_entry_point_offsets > 0) {
      TRUE_OR_RETURN(br->ReadUE(&slice_header->offset_len_minus1));
      slice_header->entry_point_offset_minus1.resize(
          slice_header->num_entry_point_offsets);
      for (int i = 0; i < slice_header->num_entry_point_offsets; i++) {
        TRUE_OR_RETURN(
            br->ReadBits(slice_header->offset_len_minus1 + 1,
                         &slice_header->entry_point_offset_minus1[i]));
      }
    }
  }

  if (pps->slice_segment_header_extension_present_flag) {
    int extension_length;
    TRUE_OR_RETURN(br->ReadUE(&extension_length));
    TRUE_OR_RETURN(br->SkipBits(extension_length * 8));
  }

  OK_OR_RETURN(ByteAlignment(br));

  slice_header->header_bit_size = nalu.payload_size() * 8 - br->NumBitsLeft();
  return kOk;
}

H265Parser::Result H265Parser::ParsePps(const Nalu& nalu, int* pps_id) {
  DCHECK_EQ(Nalu::H265_PPS, nalu.type());

  // Reads most of the element, not reading the extension data.
  H26xBitReader reader;
  reader.Initialize(nalu.data() + nalu.header_size(), nalu.payload_size());
  H26xBitReader* br = &reader;

  *pps_id = -1;
  std::unique_ptr<H265Pps> pps(new H265Pps);

  TRUE_OR_RETURN(br->ReadUE(&pps->pic_parameter_set_id));
  TRUE_OR_RETURN(br->ReadUE(&pps->seq_parameter_set_id));

  TRUE_OR_RETURN(br->ReadBool(&pps->dependent_slice_segments_enabled_flag));
  TRUE_OR_RETURN(br->ReadBool(&pps->output_flag_present_flag));
  TRUE_OR_RETURN(br->ReadBits(3, &pps->num_extra_slice_header_bits));
  TRUE_OR_RETURN(br->ReadBool(&pps->sign_data_hiding_enabled_flag));
  TRUE_OR_RETURN(br->ReadBool(&pps->cabac_init_present_flag));

  TRUE_OR_RETURN(br->ReadUE(&pps->num_ref_idx_l0_default_active_minus1));
  TRUE_OR_RETURN(br->ReadUE(&pps->num_ref_idx_l1_default_active_minus1));
  TRUE_OR_RETURN(br->ReadSE(&pps->init_qp_minus26));
  TRUE_OR_RETURN(br->ReadBool(&pps->constrained_intra_pred_flag));
  TRUE_OR_RETURN(br->ReadBool(&pps->transform_skip_enabled_flag));

  TRUE_OR_RETURN(br->ReadBool(&pps->cu_qp_delta_enabled_flag));
  if (pps->cu_qp_delta_enabled_flag)
    TRUE_OR_RETURN(br->ReadUE(&pps->diff_cu_qp_delta_depth));
  TRUE_OR_RETURN(br->ReadSE(&pps->cb_qp_offset));
  TRUE_OR_RETURN(br->ReadSE(&pps->cr_qp_offset));

  TRUE_OR_RETURN(br->ReadBool(&pps->slice_chroma_qp_offsets_present_flag));
  TRUE_OR_RETURN(br->ReadBool(&pps->weighted_pred_flag));
  TRUE_OR_RETURN(br->ReadBool(&pps->weighted_bipred_flag));
  TRUE_OR_RETURN(br->ReadBool(&pps->transquant_bypass_enabled_flag));
  TRUE_OR_RETURN(br->ReadBool(&pps->tiles_enabled_flag));
  TRUE_OR_RETURN(br->ReadBool(&pps->entropy_coding_sync_enabled_flag));

  if (pps->tiles_enabled_flag) {
    TRUE_OR_RETURN(br->ReadUE(&pps->num_tile_columns_minus1));
    TRUE_OR_RETURN(br->ReadUE(&pps->num_tile_rows_minus1));
    TRUE_OR_RETURN(br->ReadBool(&pps->uniform_spacing_flag));
    if (!pps->uniform_spacing_flag) {
      pps->column_width_minus1.resize(pps->num_tile_columns_minus1);
      for (int i = 0; i < pps->num_tile_columns_minus1; i++) {
        TRUE_OR_RETURN(br->ReadUE(&pps->column_width_minus1[i]));
      }
      pps->row_height_minus1.resize(pps->num_tile_rows_minus1);
      for (int i = 0; i < pps->num_tile_rows_minus1; i++) {
        TRUE_OR_RETURN(br->ReadUE(&pps->row_height_minus1[i]));
      }
    }
    TRUE_OR_RETURN(br->ReadBool(&pps->loop_filter_across_tiles_enabled_flag));
  }

  TRUE_OR_RETURN(br->ReadBool(&pps->loop_filter_across_slices_enabled_flag));
  TRUE_OR_RETURN(br->ReadBool(&pps->deblocking_filter_control_present_flag));
  if (pps->deblocking_filter_control_present_flag) {
    TRUE_OR_RETURN(br->ReadBool(&pps->deblocking_filter_override_enabled_flag));
    TRUE_OR_RETURN(br->ReadBool(&pps->deblocking_filter_disabled_flag));
    if (!pps->deblocking_filter_disabled_flag) {
      TRUE_OR_RETURN(br->ReadSE(&pps->beta_offset_div2));
      TRUE_OR_RETURN(br->ReadSE(&pps->tc_offset_div2));
    }
  }

  TRUE_OR_RETURN(br->ReadBool(&pps->scaling_list_data_present_flag));
  if (pps->scaling_list_data_present_flag) {
    OK_OR_RETURN(SkipScalingListData(br));
  }

  TRUE_OR_RETURN(br->ReadBool(&pps->lists_modification_present_flag));
  TRUE_OR_RETURN(br->ReadUE(&pps->log2_parallel_merge_level_minus2));

  TRUE_OR_RETURN(
      br->ReadBool(&pps->slice_segment_header_extension_present_flag));

  bool pps_extension_present_flag;
  bool pps_range_extension_flag = false;
  TRUE_OR_RETURN(br->ReadBool(&pps_extension_present_flag));
  if (pps_extension_present_flag) {
    TRUE_OR_RETURN(br->ReadBool(&pps_range_extension_flag));
    // pps_multilayer_extension_flag, pps_3d_extension_flag, pps_extension_5bits
    TRUE_OR_RETURN(br->SkipBits(1 + 1 + 5));
  }

  if (pps_range_extension_flag) {
    if (pps->transform_skip_enabled_flag) {
      // log2_max_transform_skip_block_size_minus2
      int ignored;
      TRUE_OR_RETURN(br->ReadUE(&ignored));
    }

    TRUE_OR_RETURN(br->SkipBits(1));  // cross_component_prediction_enabled_flag
    TRUE_OR_RETURN(br->ReadBool(&pps->chroma_qp_offset_list_enabled_flag));
    // Incomplete
  }

  // Ignore remaining extension data.

  // This will replace any existing PPS instance.
  *pps_id = pps->pic_parameter_set_id;
  active_ppses_[*pps_id] = std::move(pps);

  return kOk;
}

H265Parser::Result H265Parser::ParseSps(const Nalu& nalu, int* sps_id) {
  DCHECK_EQ(Nalu::H265_SPS, nalu.type());

  // Reads most of the element, not reading the extension data.
  H26xBitReader reader;
  reader.Initialize(nalu.data() + nalu.header_size(), nalu.payload_size());
  H26xBitReader* br = &reader;

  *sps_id = -1;

  std::unique_ptr<H265Sps> sps(new H265Sps);

  TRUE_OR_RETURN(br->ReadBits(4, &sps->video_parameter_set_id));
  TRUE_OR_RETURN(br->ReadBits(3, &sps->max_sub_layers_minus1));
  const bool multi_layer_ext_sps_flag =
      nalu.nuh_layer_id() != 0 && sps->max_sub_layers_minus1 == 7;

  const H265Vps* vps = active_vpses_[sps->video_parameter_set_id].get();

  int layer_id_in_vps = 0;
  if (vps != nullptr && vps->vps_max_layers_minus1 > 0) {
    layer_id_in_vps = vps->layer_id_in_vps[nalu.nuh_layer_id()];
  }
  if (!multi_layer_ext_sps_flag) {
    TRUE_OR_RETURN(br->ReadBool(&sps->temporal_id_nesting_flag));

    OK_OR_RETURN(ReadProfileTierLevel(true, sps->max_sub_layers_minus1, br,
                                      nullptr,
                                      sps->general_profile_tier_level_data));
  } else {
    // VPS is needed in this case.
    TRUE_OR_RETURN(vps != nullptr);

    // Get profile/tier/level from the VPS.
    // Assume the last (output) layer set is the target output layer set.
    const int profile_tier_level_idx =
        vps->profile_tier_level_idx[vps->vps_num_layer_sets_minus1]
                                   [layer_id_in_vps];
    TRUE_OR_RETURN(profile_tier_level_idx < vps->num_profile_tier_levels);
    memcpy(sps->general_profile_tier_level_data,
           vps->general_profile_tier_level_data[profile_tier_level_idx],
           12 * sizeof(sps->general_profile_tier_level_data[0]));
  }

  TRUE_OR_RETURN(br->ReadUE(&sps->seq_parameter_set_id));

  if (multi_layer_ext_sps_flag) {
    int sps_rep_format_idx = 0;
    bool update_rep_format_flag;
    TRUE_OR_RETURN(br->ReadBool(&update_rep_format_flag));
    if (update_rep_format_flag) {
      TRUE_OR_RETURN(br->ReadBits(8, &sps_rep_format_idx));
      // Currently only one rep_format() is supported in the VPS
      TRUE_OR_RETURN(sps_rep_format_idx == 0);
    }
    const H265RepFormat* rep_format = &vps->rep_format[sps_rep_format_idx];
    sps->chroma_format_idc = rep_format->chroma_format_vps_idc;
    sps->separate_colour_plane_flag =
        rep_format->separate_colour_plane_vps_flag;
    sps->pic_width_in_luma_samples = rep_format->pic_width_vps_in_luma_samples;
    sps->pic_height_in_luma_samples =
        rep_format->pic_height_vps_in_luma_samples;
    sps->conf_win_left_offset = rep_format->conf_win_vps_left_offset;
    sps->conf_win_right_offset = rep_format->conf_win_vps_right_offset;
    sps->conf_win_top_offset = rep_format->conf_win_vps_top_offset;
    sps->conf_win_bottom_offset = rep_format->conf_win_vps_bottom_offset;
    sps->bit_depth_luma_minus8 = rep_format->bit_depth_vps_luma_minus8;
    sps->bit_depth_chroma_minus8 = rep_format->bit_depth_vps_chroma_minus8;
  } else {
    TRUE_OR_RETURN(br->ReadUE(&sps->chroma_format_idc));
    if (sps->chroma_format_idc == 3) {
      TRUE_OR_RETURN(br->ReadBool(&sps->separate_colour_plane_flag));
    }
    TRUE_OR_RETURN(br->ReadUE(&sps->pic_width_in_luma_samples));
    TRUE_OR_RETURN(br->ReadUE(&sps->pic_height_in_luma_samples));

    TRUE_OR_RETURN(br->ReadBool(&sps->conformance_window_flag));
    if (sps->conformance_window_flag) {
      TRUE_OR_RETURN(br->ReadUE(&sps->conf_win_left_offset));
      TRUE_OR_RETURN(br->ReadUE(&sps->conf_win_right_offset));
      TRUE_OR_RETURN(br->ReadUE(&sps->conf_win_top_offset));
      TRUE_OR_RETURN(br->ReadUE(&sps->conf_win_bottom_offset));
    }

    TRUE_OR_RETURN(br->ReadUE(&sps->bit_depth_luma_minus8));
    TRUE_OR_RETURN(br->ReadUE(&sps->bit_depth_chroma_minus8));
  }
  TRUE_OR_RETURN(br->ReadUE(&sps->log2_max_pic_order_cnt_lsb_minus4));

  if (!multi_layer_ext_sps_flag) {
    TRUE_OR_RETURN(br->ReadBool(&sps->sub_layer_ordering_info_present_flag));
    int start = sps->sub_layer_ordering_info_present_flag
                    ? 0
                    : sps->max_sub_layers_minus1;
    for (int i = start; i <= sps->max_sub_layers_minus1; i++) {
      TRUE_OR_RETURN(br->ReadUE(&sps->max_dec_pic_buffering_minus1[i]));
      TRUE_OR_RETURN(br->ReadUE(&sps->max_num_reorder_pics[i]));
      TRUE_OR_RETURN(br->ReadUE(&sps->max_latency_increase_plus1[i]));
    }
  }
  TRUE_OR_RETURN(br->ReadUE(&sps->log2_min_luma_coding_block_size_minus3));
  TRUE_OR_RETURN(br->ReadUE(&sps->log2_diff_max_min_luma_coding_block_size));
  TRUE_OR_RETURN(br->ReadUE(&sps->log2_min_luma_transform_block_size_minus2));
  TRUE_OR_RETURN(br->ReadUE(&sps->log2_diff_max_min_luma_transform_block_size));
  TRUE_OR_RETURN(br->ReadUE(&sps->max_transform_hierarchy_depth_inter));
  TRUE_OR_RETURN(br->ReadUE(&sps->max_transform_hierarchy_depth_intra));

  TRUE_OR_RETURN(br->ReadBool(&sps->scaling_list_enabled_flag));
  if (sps->scaling_list_enabled_flag) {
    bool sps_infer_scaling_list_flag = false;
    if (multi_layer_ext_sps_flag) {
      TRUE_OR_RETURN(br->ReadBool(&sps_infer_scaling_list_flag));
    }
    if (sps_infer_scaling_list_flag) {
      TRUE_OR_RETURN(br->SkipBits(6));  // sps_scaling_list_ref_layer_id
    } else {
      TRUE_OR_RETURN(br->ReadBool(&sps->scaling_list_data_present_flag));
      if (sps->scaling_list_data_present_flag) {
        OK_OR_RETURN(SkipScalingListData(br));
      }
    }
  }

  TRUE_OR_RETURN(br->ReadBool(&sps->amp_enabled_flag));
  TRUE_OR_RETURN(br->ReadBool(&sps->sample_adaptive_offset_enabled_flag));
  TRUE_OR_RETURN(br->ReadBool(&sps->pcm_enabled_flag));
  if (sps->pcm_enabled_flag) {
    TRUE_OR_RETURN(br->ReadBits(4, &sps->pcm_sample_bit_depth_luma_minus1));
    TRUE_OR_RETURN(br->ReadBits(4, &sps->pcm_sample_bit_depth_chroma_minus1));
    TRUE_OR_RETURN(
        br->ReadUE(&sps->log2_min_pcm_luma_coding_block_size_minus3));
    TRUE_OR_RETURN(
        br->ReadUE(&sps->log2_diff_max_min_pcm_luma_coding_block_size));
    TRUE_OR_RETURN(br->ReadBool(&sps->pcm_loop_filter_disabled_flag));
  }

  TRUE_OR_RETURN(br->ReadUE(&sps->num_short_term_ref_pic_sets));
  sps->st_ref_pic_sets.resize(sps->num_short_term_ref_pic_sets);
  for (int i = 0; i < sps->num_short_term_ref_pic_sets; i++) {
    OK_OR_RETURN(ParseReferencePictureSet(sps->num_short_term_ref_pic_sets, i,
                                          sps->st_ref_pic_sets, br,
                                          &sps->st_ref_pic_sets[i]));
  }

  TRUE_OR_RETURN(br->ReadBool(&sps->long_term_ref_pic_present_flag));
  if (sps->long_term_ref_pic_present_flag) {
    TRUE_OR_RETURN(br->ReadUE(&sps->num_long_term_ref_pics));
    sps->lt_ref_pic_poc_lsb.resize(sps->num_long_term_ref_pics);
    sps->used_by_curr_pic_lt_flag.resize(sps->num_long_term_ref_pics);
    for (int i = 0; i < sps->num_long_term_ref_pics; i++) {
      TRUE_OR_RETURN(br->ReadBits(sps->log2_max_pic_order_cnt_lsb_minus4 + 4,
                                  &sps->lt_ref_pic_poc_lsb[i]));
      bool temp;
      TRUE_OR_RETURN(br->ReadBool(&temp));
      sps->used_by_curr_pic_lt_flag[i] = temp;
    }
  }

  TRUE_OR_RETURN(br->ReadBool(&sps->temporal_mvp_enabled_flag));
  TRUE_OR_RETURN(br->ReadBool(&sps->strong_intra_smoothing_enabled_flag));

  TRUE_OR_RETURN(br->ReadBool(&sps->vui_parameters_present));
  if (sps->vui_parameters_present) {
    OK_OR_RETURN(ParseVuiParameters(sps->max_sub_layers_minus1, br,
                                    &sps->vui_parameters));
  }

  // Ignore remaining extension data.

  // This will replace any existing SPS instance.
  *sps_id = sps->seq_parameter_set_id;
  active_spses_[*sps_id] = std::move(sps);

  return kOk;
}

H265Parser::Result H265Parser::ParseVps(const Nalu& nalu, int* vps_id) {
  DCHECK_EQ(Nalu::H265_VPS, nalu.type());

  // Reads only the data needed.
  H26xBitReader reader;
  reader.Initialize(nalu.data() + nalu.header_size(), nalu.payload_size());
  H26xBitReader* br = &reader;

  bool temp_bool;
  int temp_int;

  std::unique_ptr<H265Vps> vps(new H265Vps);

  *vps_id = -1;
  TRUE_OR_RETURN(br->ReadBits(4, &vps->vps_video_parameter_set_id));

  TRUE_OR_RETURN(br->ReadBool(&vps->vps_base_layer_internal_flag));
  TRUE_OR_RETURN(br->ReadBool(&vps->vps_base_layer_available_flag));

  TRUE_OR_RETURN(br->ReadBits(6, &vps->vps_max_layers_minus1));
  TRUE_OR_RETURN(vps->vps_max_layers_minus1 + 1 <= kMaxLayers);

  TRUE_OR_RETURN(br->ReadBits(3, &vps->vps_max_sub_layers_minus1));

  // vps_temporal_id_nesting_flag, vps_reserved_0xffff_16bits
  TRUE_OR_RETURN(br->SkipBits(17));

  OK_OR_RETURN(
      ReadProfileTierLevel(true, vps->vps_max_sub_layers_minus1, br, nullptr,
                           vps.get()->general_profile_tier_level_data[0]));
  vps->num_profile_tier_levels = 1;

  // vps_sub_layer_ordering_info_present_flag
  TRUE_OR_RETURN(br->ReadBool(&temp_bool));
  int start = temp_bool ? 0 : vps->vps_max_sub_layers_minus1;
  for (int i = start; i <= vps->vps_max_sub_layers_minus1; i++) {
    // vps_max_dec_pic_buffering_minus1[i], vps_max_num_reorder_pics[i],
    // vps_max_latency_increase_plus1[i]
    TRUE_OR_RETURN(br->ReadUE(&temp_int));
    TRUE_OR_RETURN(br->ReadUE(&temp_int));
    TRUE_OR_RETURN(br->ReadUE(&temp_int));
  }

  TRUE_OR_RETURN(br->ReadBits(6, &vps->vps_max_layer_id));
  TRUE_OR_RETURN(br->ReadUE(&vps->vps_num_layer_sets_minus1));

  if ((vps->vps_max_layers_minus1 < 1 || vps->vps_num_layer_sets_minus1 < 1) ||
      (!vps->vps_base_layer_internal_flag ||
       !vps->vps_base_layer_internal_flag) ||
      (vps->vps_max_layer_id < vps->vps_max_layers_minus1)) {
    // 1. Must have more than 1 layer set for stereo video.
    // 2. The base layer must included within the bitstream.
    // 3. Must have enough layer ID range to support the number of layers.

    // If any one of the above conditions fails, assume just a single layer
    // video.
    vps->vps_max_layers_minus1 = 0;
    *vps_id = vps->vps_video_parameter_set_id;
    active_vpses_[*vps_id] = std::move(vps);
    return kOk;
  }

  // Define each layer set information: list of layer IDs, etc.
  int layer_set_layer_id_list[kMaxLayerSets][kMaxLayerIdPlus1];
  int num_layers_in_id_list[kMaxLayerSets];
  int layer_set_max_layer_id[kMaxLayerSets];
  // The first layer set is comprised of only the base layer.
  layer_set_layer_id_list[0][0] = 0;
  num_layers_in_id_list[0] = 1;
  layer_set_max_layer_id[0] = 0;
  // Define other layer sets.
  for (int i = 1; i <= vps->vps_num_layer_sets_minus1; i++) {
    int n = 0;
    for (int j = 0; j <= vps->vps_max_layer_id; j++) {
      bool layer_id_included_flag;
      TRUE_OR_RETURN(br->ReadBool(&layer_id_included_flag));
      if (layer_id_included_flag) {
        layer_set_layer_id_list[i][n++] = j;
        layer_set_max_layer_id[i] = j;
      }
    }
    num_layers_in_id_list[i] = n;
  }

  TRUE_OR_RETURN(br->ReadBool(&temp_bool));  // vps_timing_info_present_flag
  if (temp_bool) {
    TRUE_OR_RETURN(br->SkipBits(64));  // vps_num_units_in_tick, vps_time_scale
    TRUE_OR_RETURN(
        br->ReadBool(&temp_bool));  // vps_poc_proportional_to_timing_flag
    if (temp_bool) {
      TRUE_OR_RETURN(
          br->ReadUE(&temp_int));  // vps_num_ticks_poc_diff_one_minus1
    }
    TRUE_OR_RETURN(br->ReadUE(&temp_int));  // vps_num_hrd_parameters
    for (int i = 0; i < temp_int; i++) {
      int hrd_layer_set_idx;
      TRUE_OR_RETURN(br->ReadUE(&hrd_layer_set_idx));

      bool common_inf_present_flag = true;
      if (i != 0) {
        TRUE_OR_RETURN(br->ReadBool(&common_inf_present_flag));
      }
      OK_OR_RETURN(SkipHrdParameters(common_inf_present_flag,
                                     vps->vps_max_sub_layers_minus1, br));
    }
  }

  // For stereo video, vps_extension() needs to be parsed.
  TRUE_OR_RETURN(br->ReadBool(&temp_bool));  // vps_extension_flag
  if (!temp_bool) {
    // If no vps_extension(), then fallback to single layer HEVC.
    vps->vps_max_layers_minus1 = 0;
    *vps_id = vps->vps_video_parameter_set_id;
    active_vpses_[*vps_id] = std::move(vps);
    return kOk;
  }

  OK_OR_RETURN(ByteAlignment(br));

  OK_OR_RETURN(
      ReadProfileTierLevel(false, vps->vps_max_sub_layers_minus1, br,
                           vps.get()->general_profile_tier_level_data[0],
                           vps.get()->general_profile_tier_level_data[1]));

  bool splitting_flag;
  bool scalability_mask_flag[16];
  int num_scalability_types = 0;
  TRUE_OR_RETURN(br->ReadBool(&splitting_flag));
  for (int i = 0; i < 16; i++) {
    TRUE_OR_RETURN(br->ReadBool(&scalability_mask_flag[i]));
    if (scalability_mask_flag[i]) {
      num_scalability_types++;
    }
  }

  // As listed in Table F.1 of the spec, num_scalability_types indicates the
  // number of different scalability dimensions.  If there is no scalability
  // dimension, then we simply have a single-layer HEVC.  Currently, only 2
  // view multi-view coding is supported; in other cases, simply fallback to
  // single-layer HEVC.
  if (num_scalability_types == 0 ||
      num_scalability_types > kMaxScalabilityTypes ||
      !scalability_mask_flag[1]) {
    vps->vps_max_sub_layers_minus1 = 0;
    *vps_id = vps->vps_video_parameter_set_id;
    active_vpses_[*vps_id] = std::move(vps);
    return kOk;
  }

  vps->scalability_type = H265Vps::kMultiview;

  int dimension_id_len_minus1[kMaxScalabilityTypes];
  for (int i = 0; i < num_scalability_types - (splitting_flag ? 1 : 0); i++) {
    TRUE_OR_RETURN(br->ReadBits(3, &dimension_id_len_minus1[i]));
  }

  int dim_bit_offset[kMaxScalabilityTypes + 1];
  dim_bit_offset[0] = 0;
  if (splitting_flag) {
    for (int i = 1; i < num_scalability_types; i++) {
      for (int j = 0; j < i; j++) {
        dim_bit_offset[i] += dimension_id_len_minus1[j] + 1;
      }
    }
    dim_bit_offset[num_scalability_types] = 6;
  }

  int dimension_id[kMaxLayers][kMaxScalabilityTypes];
  for (int j = 0; j < num_scalability_types; j++) {
    dimension_id[0][j] = 0;
  }

  // Get layer_id_in_nuh that maps the layer ID used in this VPS to the NAL
  // unit header's parsed layer ID - nuh_layer_id.
  int layer_id_in_nuh[kMaxLayers];
  layer_id_in_nuh[0] = 0;

  TRUE_OR_RETURN(br->ReadBool(&temp_bool));  // vps_nuh_layer_id_present_flag
  for (int i = 1; i <= vps->vps_max_layers_minus1; i++) {
    if (temp_bool) {
      TRUE_OR_RETURN(br->ReadBits(6, &layer_id_in_nuh[i]));
    } else {
      layer_id_in_nuh[i] = i;
    }
    if (!splitting_flag) {
      for (int j = 0; j < num_scalability_types; j++) {
        TRUE_OR_RETURN(
            br->ReadBits(dimension_id_len_minus1[j] + 1, &dimension_id[i][j]));
      }
    } else {
      for (int j = 0; j < num_scalability_types; j++) {
        dimension_id[i][j] =
            (layer_id_in_nuh[i] & ((1 << dim_bit_offset[j + 1]) - 1)) >>
            dim_bit_offset[j];
      }
    }
  }

  // Derive view_order_idx[] and num_views following (F-3) in
  // Subsection F.7.4.3.1.1 of the standard.
  int view_order_idx[kMaxLayerIdPlus1];
  vps->num_views = 1;
  for (int i = 0; i <= vps->vps_max_layers_minus1; i++) {
    view_order_idx[layer_id_in_nuh[i]] = -1;
    for (int sm_idx = 0, j = 0; sm_idx < 16; sm_idx++) {
      if (scalability_mask_flag[sm_idx]) {
        if (sm_idx == 1) {  // multiview
          // Note that view_order_idx is expected to be an index as it is used
          // to access view_id_val[]; however, dimension_id[i][j] is not
          // expected to follow the index constraint.  It is up to the encoder
          // to ensure that the dimension_id[i][j] is consistent with the use
          // of view_order_idx.
          view_order_idx[layer_id_in_nuh[i]] = dimension_id[i][j];
        }
        j++;
      }
    }
    if (i > 0) {
      bool new_view = true;
      for (int j = 0; j < i; j++) {
        if (view_order_idx[layer_id_in_nuh[i]] ==
            view_order_idx[layer_id_in_nuh[j]]) {
          new_view = false;
          break;
        }
      }
      if (new_view) {
        vps->num_views++;
      }
    }
  }

  int view_id_len;
  TRUE_OR_RETURN(br->ReadBits(4, &view_id_len));
  if (vps->num_views != kNumViews || view_id_len == 0) {
    NOTIMPLEMENTED() << "Only multiview extension with 2 views is supported.";
    return kUnsupportedStream;
  }

  int view_id_val[kNumViews];
  for (int i = 0; i < vps->num_views; i++) {
    TRUE_OR_RETURN(br->ReadBits(view_id_len, &view_id_val[i]));
  }

  for (int i = 0; i <= vps->vps_max_layers_minus1; i++) {
    vps->layer_id_in_vps[layer_id_in_nuh[i]] = i;
  }
  for (int i = 0; i <= vps->vps_max_layer_id; i++) {
    int view_id_val_idx = std::min(view_order_idx[i], vps->num_views - 1);
    vps->view_id[vps->layer_id_in_vps[i]] =
        view_id_val_idx >= 0 ? view_id_val[view_id_val_idx] : kInvalidId;
  }

  // Derive H.265 layer dependency structure following (F-4) in
  // Subsection F.7.4.3.1.1 of the standard.
  bool direct_dependency_flag[kMaxLayers][kMaxLayers];
  bool dependency_flag[kMaxLayers][kMaxLayers];
  for (int i = 1; i <= vps->vps_max_layers_minus1; i++) {
    for (int j = 0; j < i; j++) {
      TRUE_OR_RETURN(br->ReadBool(&direct_dependency_flag[i][j]));
      dependency_flag[i][j] = direct_dependency_flag[i][j];
    }
  }
  for (int i = 1; i <= vps->vps_max_layers_minus1; i++) {
    for (int j = 0; j < vps->vps_max_layers_minus1; j++) {
      for (int k = 0; k < i; k++) {
        if (dependency_flag[i][k] && dependency_flag[k][j]) {
          dependency_flag[i][j] = true;
          break;
        }
      }
    }
  }

  // Derive num_direct_ref_layers following (F-5) in Subsection F.7.4.3.1.1.
  for (int i = 1; i <= vps->vps_max_layers_minus1; i++) {
    int d = 0;
    for (int j = 0; j < i; j++) {
      if (direct_dependency_flag[i][j]) {
        vps->id_direct_ref_layers[layer_id_in_nuh[i]][d++] = layer_id_in_nuh[j];
      }
    }
    vps->num_direct_ref_layers[layer_id_in_nuh[i]] = d;
  }
  // Derive num_independent_layers following (F-6) in Subsection F.7.4.3.1.1.
  int num_independent_layers = 0;
  for (int i = 0; i <= vps->vps_max_layers_minus1; i++) {
    if (vps->num_direct_ref_layers[layer_id_in_nuh[i]] == 0) {
      num_independent_layers++;
    }
  }

  if (num_independent_layers > 1) {
    NOTIMPLEMENTED() << "Only single independent layer is supported.";
    return kUnsupportedStream;
  }

  // Since num_independent_layers <= 1, no need to parse num_add_layer_sets or
  // highest_layer_idx_plus1.

  TRUE_OR_RETURN(
      br->ReadBool(&temp_bool));  // vps_sub_layers_max_minus1_present_flag
  if (temp_bool) {
    for (int i = 0; i <= vps->vps_max_layers_minus1; i++) {
      TRUE_OR_RETURN(br->ReadBits(3, &vps->sub_layers_vps_max_minus1[i]));
    }
  } else {
    for (int i = 0; i <= vps->vps_max_layers_minus1; i++) {
      vps->sub_layers_vps_max_minus1[i] = vps->vps_max_sub_layers_minus1;
    }
  }

  TRUE_OR_RETURN(br->ReadBool(&temp_bool));  // max_tid_ref_present_flag
  if (temp_bool) {
    for (int i = 0; i <= vps->vps_max_layers_minus1; i++) {
      for (int j = i + 1; j <= vps->vps_max_layers_minus1; j++) {
        if (direct_dependency_flag[j][i]) {
          TRUE_OR_RETURN(
              br->ReadBits(3, &vps->max_tid_il_ref_pics_plus1[i][j]));
        } else {
          vps->max_tid_il_ref_pics_plus1[i][j] = 7;
        }
      }
    }
  } else {
    for (int i = 0; i <= vps->vps_max_layers_minus1; i++) {
      for (int j = i + 1; j <= vps->vps_max_layers_minus1; j++) {
        vps->max_tid_il_ref_pics_plus1[i][j] = 7;
      }
    }
  }

  TRUE_OR_RETURN(br->ReadBool(&vps->default_ref_layers_active_flag));

  // Get profile_tier_level()s needed for non-base layer.
  TRUE_OR_RETURN(br->ReadUE(&temp_int));  // vps_num_profile_tier_level_minus1
  vps->num_profile_tier_levels = temp_int + 1;
  if (vps->num_profile_tier_levels > kMaxNumProfileTierLevels) {
    NOTIMPLEMENTED()
        << "Only up to " << kMaxNumProfileTierLevels
        << " profile_tier_levels are supported in the L-HEVC case.";
    return kUnsupportedStream;
  }
  // Note that vps_base_layer_internal_flag is always true, so i starts from 2
  if (vps->num_profile_tier_levels > 2) {
    for (int i = 2; i < vps->num_profile_tier_levels; i++) {
      TRUE_OR_RETURN(br->ReadBool(&temp_bool));  // vps_profile_present_flag[i]
      OK_OR_RETURN(ReadProfileTierLevel(
          temp_bool, vps->vps_max_sub_layers_minus1, br,
          vps.get()->general_profile_tier_level_data[i - 1],
          vps.get()->general_profile_tier_level_data[i]));
    }
  }

  TRUE_OR_RETURN(br->ReadUE(&temp_int));  // num_add_olss
  if (temp_int > 0) {
    NOTIMPLEMENTED() << "No additional output layer sets are supported.";
    return kUnsupportedStream;
  }
  int num_output_layer_sets = vps->vps_num_layer_sets_minus1 + 1 + temp_int;

  int default_output_layer_idc;
  TRUE_OR_RETURN(br->ReadBits(2, &default_output_layer_idc));

  bool output_layer_flag[kMaxOuputLayerSets][kMaxLayerIdPlus1];
  int num_output_layers_in_output_layer_set[kMaxOuputLayerSets];
  int ols_highest_output_layer_id[kMaxOuputLayerSets];
  for (int i = 0; i <= vps->vps_num_layer_sets_minus1; i++) {
    num_output_layers_in_output_layer_set[i] = 0;
    ols_highest_output_layer_id[i] = layer_set_max_layer_id[i];
    if (default_output_layer_idc == 0) {
      for (int j = 0; j < num_layers_in_id_list[i]; j++) {
        output_layer_flag[i][j] = true;
      }
      num_output_layers_in_output_layer_set[i] = num_layers_in_id_list[i];
    } else if (default_output_layer_idc == 1) {
      for (int j = 0; j < num_layers_in_id_list[i]; j++) {
        output_layer_flag[i][j] =
            layer_set_layer_id_list[i][j] == layer_set_max_layer_id[i];
      }
      num_output_layers_in_output_layer_set[i] = 1;
    } else {
      output_layer_flag[0][0] = true;
      num_output_layers_in_output_layer_set[0] = 1;
    }
  }

  // Below simplified as num_output_layer_sets == vps_num_layer_sets_minus1+1.
  for (int i = 1; i < num_output_layer_sets; i++) {
    if (default_output_layer_idc == 2) {
      for (int j = 0; j < num_layers_in_id_list[i]; j++) {
        TRUE_OR_RETURN(br->ReadBool(&output_layer_flag[i][j]));
        if (output_layer_flag[i][j]) {
          num_output_layers_in_output_layer_set[i] += 1;
          ols_highest_output_layer_id[i] = layer_set_layer_id_list[i][j];
        }
      }
    }

    for (int j = 0; j < num_layers_in_id_list[i]; j++) {
      if (vps->num_profile_tier_levels > 1) {
        bool necessary_layer_flag = output_layer_flag[i][j];
        int bit_len = ceil(log2(vps->num_profile_tier_levels));
        if (!necessary_layer_flag) {
          int curr_layer_id_in_vps =
              vps->layer_id_in_vps[layer_set_layer_id_list[i][j]];
          for (int k = 0; k < j; k++) {
            int ref_layer_id_in_vps =
                vps->layer_id_in_vps[layer_set_layer_id_list[i][k]];
            if (dependency_flag[curr_layer_id_in_vps][ref_layer_id_in_vps]) {
              necessary_layer_flag = true;
              break;
            }
          }
        }
        if (necessary_layer_flag) {
          TRUE_OR_RETURN(
              br->ReadBits(bit_len, &vps->profile_tier_level_idx[i][j]));
        }
      }
    }
    if (num_output_layers_in_output_layer_set[i] == 1 &&
        vps->num_direct_ref_layers[ols_highest_output_layer_id[i]] > 0) {
      TRUE_OR_RETURN(br->SkipBits(1));  // alt_output_layer_flag[i]
    }
  }

  TRUE_OR_RETURN(br->ReadUE(&temp_int));  // vps_num_rep_formats_minus1
  vps->num_rep_formats = temp_int + 1;
  if (vps->num_rep_formats > kMaxNumRepFromats) {
    NOTIMPLEMENTED() << "Only up to " << kMaxNumRepFromats
                     << " rep_formats are supported.";
    return kUnsupportedStream;
  }

  for (int i = 0; i < vps->num_rep_formats; i++) {
    // Parse rep_format().
    H265RepFormat* rep_format = &vps->rep_format[i];
    TRUE_OR_RETURN(
        br->ReadBits(16, &rep_format->pic_width_vps_in_luma_samples));
    TRUE_OR_RETURN(
        br->ReadBits(16, &rep_format->pic_height_vps_in_luma_samples));
    TRUE_OR_RETURN(
        br->ReadBool(&temp_bool));  // chroma_and_bit_depth_vps_present_flag
    if (temp_bool) {
      TRUE_OR_RETURN(br->ReadBits(2, &rep_format->chroma_format_vps_idc));
      if (rep_format->chroma_format_vps_idc == 3) {
        TRUE_OR_RETURN(
            br->ReadBool(&rep_format->separate_colour_plane_vps_flag));
      }
      TRUE_OR_RETURN(br->ReadBits(4, &rep_format->bit_depth_vps_luma_minus8));
      TRUE_OR_RETURN(br->ReadBits(4, &rep_format->bit_depth_vps_luma_minus8));
    }
    TRUE_OR_RETURN(br->ReadBool(&temp_bool));  // conformance_window_vps_flag
    if (temp_bool) {
      TRUE_OR_RETURN(br->ReadUE(&rep_format->conf_win_vps_left_offset));
      TRUE_OR_RETURN(br->ReadUE(&rep_format->conf_win_vps_right_offset));
      TRUE_OR_RETURN(br->ReadUE(&rep_format->conf_win_vps_top_offset));
      TRUE_OR_RETURN(br->ReadUE(&rep_format->conf_win_vps_bottom_offset));
    }
  }

  // Since kMaxNumRepFromats and vps_num_rep_formats_minus1 == 0, no need to
  // parse rep_format_idx_present_flag.  Since rep_format_idx_present_flag is
  // inferred to be 0, no need to parse vps_rep_format_idx[i].

  TRUE_OR_RETURN(br->ReadBool(&vps->max_one_active_ref_layer_flag));
  // For stereo views, the secondary view only depends on the primary view, so
  // at most 1 ref layer is needed!
  TRUE_OR_RETURN(vps->max_one_active_ref_layer_flag);

  TRUE_OR_RETURN(br->ReadBool(&temp_bool));  // vps_poc_lsb_aligned_flag

  for (int i = 1; i <= vps->vps_max_layers_minus1; i++) {
    if (vps->num_direct_ref_layers[layer_id_in_nuh[i]] == 0) {
      TRUE_OR_RETURN(br->ReadBool(&vps->poc_lsb_not_present_flag[i]));
    } else {
      vps->poc_lsb_not_present_flag[i] = false;
    }
  }

  // Ignore remaining data.

  *vps_id = vps->vps_video_parameter_set_id;

  active_vpses_[*vps_id] = std::move(vps);

  return kOk;
}

const H265Pps* H265Parser::GetPps(int pps_id) {
  return active_ppses_[pps_id].get();
}

const H265Sps* H265Parser::GetSps(int sps_id) {
  return active_spses_[sps_id].get();
}

const H265Vps* H265Parser::GetVps(int vps_id) {
  return active_vpses_[vps_id].get();
}

H265Parser::Result H265Parser::ParseVuiParameters(int max_num_sub_layers_minus1,
                                                  H26xBitReader* br,
                                                  H265VuiParameters* vui) {
  // Reads whole element but ignores most of it.
  int ignored;

  TRUE_OR_RETURN(br->ReadBool(&vui->aspect_ratio_info_present_flag));
  if (vui->aspect_ratio_info_present_flag) {
    TRUE_OR_RETURN(br->ReadBits(8, &vui->aspect_ratio_idc));
    if (vui->aspect_ratio_idc == H265VuiParameters::kExtendedSar) {
      TRUE_OR_RETURN(br->ReadBits(16, &vui->sar_width));
      TRUE_OR_RETURN(br->ReadBits(16, &vui->sar_height));
    }
  }

  bool overscan_info_present_flag;
  TRUE_OR_RETURN(br->ReadBool(&overscan_info_present_flag));
  if (overscan_info_present_flag) {
    TRUE_OR_RETURN(br->SkipBits(1));  // overscan_appropriate_flag
  }

  bool video_signal_type_present_flag;
  TRUE_OR_RETURN(br->ReadBool(&video_signal_type_present_flag));
  if (video_signal_type_present_flag) {
    TRUE_OR_RETURN(br->SkipBits(3));  // video_format
    TRUE_OR_RETURN(br->SkipBits(1));  // video_full_range_flag

    bool colour_description_present_flag;
    TRUE_OR_RETURN(br->ReadBool(&colour_description_present_flag));
    if (colour_description_present_flag) {
      TRUE_OR_RETURN(
          br->ReadBits(8, &vui->color_primaries));  // color_primaries
      TRUE_OR_RETURN(br->ReadBits(8, &vui->transfer_characteristics));
      TRUE_OR_RETURN(
          br->ReadBits(8, &vui->matrix_coefficients));  // matrix_coeffs
    }
  }

  bool chroma_loc_info_present_flag;
  TRUE_OR_RETURN(br->ReadBool(&chroma_loc_info_present_flag));
  if (chroma_loc_info_present_flag) {
    // chroma_sample_log_type_top_field, chroma_sample_log_type_bottom_field
    TRUE_OR_RETURN(br->ReadUE(&ignored));
    TRUE_OR_RETURN(br->ReadUE(&ignored));
  }

  // neutral_chroma_indication_flag, field_seq_flag,
  // frame_field_info_present_flag.
  TRUE_OR_RETURN(br->SkipBits(3));

  bool default_display_window_flag;
  TRUE_OR_RETURN(br->ReadBool(&default_display_window_flag));
  if (default_display_window_flag) {
    TRUE_OR_RETURN(br->ReadUE(&ignored));  // def_disp_win_left_offset
    TRUE_OR_RETURN(br->ReadUE(&ignored));  // def_disp_win_right_offset
    TRUE_OR_RETURN(br->ReadUE(&ignored));  // def_disp_win_top_offset
    TRUE_OR_RETURN(br->ReadUE(&ignored));  // def_disp_win_bottom_offset
  }

  TRUE_OR_RETURN(br->ReadBool(&vui->vui_timing_info_present_flag));
  if (vui->vui_timing_info_present_flag) {
    READ_LONG_OR_RETURN(&vui->vui_num_units_in_tick);
    READ_LONG_OR_RETURN(&vui->vui_time_scale);

    bool vui_poc_proportional_to_timing_flag;
    TRUE_OR_RETURN(br->ReadBool(&vui_poc_proportional_to_timing_flag));
    if (vui_poc_proportional_to_timing_flag) {
      // vui_num_ticks_poc_diff_one_minus1
      TRUE_OR_RETURN(br->ReadUE(&ignored));
    }

    bool vui_hdr_parameters_present_flag;
    TRUE_OR_RETURN(br->ReadBool(&vui_hdr_parameters_present_flag));
    if (vui_hdr_parameters_present_flag) {
      OK_OR_RETURN(SkipHrdParameters(true, max_num_sub_layers_minus1, br));
    }
  }

  TRUE_OR_RETURN(br->ReadBool(&vui->bitstream_restriction_flag));
  if (vui->bitstream_restriction_flag) {
    // tiles_fixed_structure_flag, motion_vectors_over_pic_boundaries_flag,
    // restricted_ref_pic_lists_flag.
    TRUE_OR_RETURN(br->SkipBits(3));

    TRUE_OR_RETURN(br->ReadUE(&vui->min_spatial_segmentation_idc));
    TRUE_OR_RETURN(br->ReadUE(&ignored));  // max_bytes_per_pic_denom
    TRUE_OR_RETURN(br->ReadUE(&ignored));  // max_bits_per_min_cu_denum
    TRUE_OR_RETURN(br->ReadUE(&ignored));  // log2_max_mv_length_horizontal
    TRUE_OR_RETURN(br->ReadUE(&ignored));  // log2_max_mv_length_vertical
  }

  return kOk;
}

H265Parser::Result H265Parser::ParseReferencePictureSet(
    int num_short_term_ref_pic_sets,
    int st_rps_idx,
    const std::vector<H265ReferencePictureSet>& ref_pic_sets,
    H26xBitReader* br,
    H265ReferencePictureSet* out_ref_pic_set) {
  // Parses and processess a short-term reference picture set.  This needs to
  // be done since the size of this element may be dependent on previous
  // reference picture sets.

  bool inter_ref_pic_set_prediction = false;
  if (st_rps_idx != 0) {
    TRUE_OR_RETURN(br->ReadBool(&inter_ref_pic_set_prediction));
  }

  if (inter_ref_pic_set_prediction) {
    int delta_idx = 1;
    if (st_rps_idx == num_short_term_ref_pic_sets) {
      TRUE_OR_RETURN(br->ReadUE(&delta_idx));
      delta_idx++;
      TRUE_OR_RETURN(delta_idx <= st_rps_idx);
    }

    int ref_rps_idx = st_rps_idx - delta_idx;
    DCHECK_LE(0, ref_rps_idx);
    DCHECK_LT(ref_rps_idx, st_rps_idx);

    bool delta_rps_sign;
    int abs_delta_rps_minus1;
    TRUE_OR_RETURN(br->ReadBool(&delta_rps_sign));
    TRUE_OR_RETURN(br->ReadUE(&abs_delta_rps_minus1));
    int delta_rps =
        delta_rps_sign ? -(abs_delta_rps_minus1 + 1) : abs_delta_rps_minus1 + 1;

    int ref_num_delta_pocs = ref_pic_sets[ref_rps_idx].num_delta_pocs;
    std::vector<bool> used_by_curr_pic(ref_num_delta_pocs + 1);
    std::vector<bool> use_delta(ref_num_delta_pocs + 1);
    for (int j = 0; j <= ref_num_delta_pocs; j++) {
      bool temp;
      TRUE_OR_RETURN(br->ReadBool(&temp));
      used_by_curr_pic[j] = temp;

      if (!used_by_curr_pic[j]) {
        TRUE_OR_RETURN(br->ReadBool(&temp));
        use_delta[j] = temp;
      } else {
        use_delta[j] = true;
      }
    }

    int ref_num_positive_pics = ref_pic_sets[ref_rps_idx].num_positive_pics;
    int ref_num_negative_pics = ref_pic_sets[ref_rps_idx].num_negative_pics;
    int i;

    // Update list 0.
    {
      i = 0;
      for (int j = ref_num_positive_pics - 1; j >= 0; j--) {
        int d_poc = ref_pic_sets[ref_rps_idx].delta_poc_s1[j] + delta_rps;
        if (d_poc < 0 && use_delta[ref_num_negative_pics + j]) {
          out_ref_pic_set->delta_poc_s0[i] = d_poc;
          out_ref_pic_set->used_by_curr_pic_s0[i] =
              used_by_curr_pic[ref_num_negative_pics + j];
          i++;
        }
      }
      if (delta_rps < 0 && use_delta[ref_num_delta_pocs]) {
        out_ref_pic_set->delta_poc_s0[i] = delta_rps;
        out_ref_pic_set->used_by_curr_pic_s0[i] =
            used_by_curr_pic[ref_num_delta_pocs];
        i++;
      }
      for (int j = 0; j < ref_num_negative_pics; j++) {
        int d_poc = ref_pic_sets[ref_rps_idx].delta_poc_s0[j] + delta_rps;
        if (d_poc < 0 && use_delta[j]) {
          out_ref_pic_set->delta_poc_s0[i] = d_poc;
          out_ref_pic_set->used_by_curr_pic_s0[i] = used_by_curr_pic[j];
          i++;
        }
      }
      out_ref_pic_set->num_negative_pics = i;
    }

    // Update list 1.
    {
      i = 0;
      for (int j = ref_num_negative_pics - 1; j >= 0; j--) {
        int d_poc = ref_pic_sets[ref_rps_idx].delta_poc_s0[j] + delta_rps;
        if (d_poc > 0 && use_delta[j]) {
          out_ref_pic_set->delta_poc_s1[i] = d_poc;
          out_ref_pic_set->used_by_curr_pic_s1[i] = used_by_curr_pic[j];
          i++;
        }
      }
      if (delta_rps > 0 && use_delta[ref_num_delta_pocs]) {
        out_ref_pic_set->delta_poc_s1[i] = delta_rps;
        out_ref_pic_set->used_by_curr_pic_s1[i] =
            used_by_curr_pic[ref_num_delta_pocs];
        i++;
      }
      for (int j = 0; j < ref_num_positive_pics; j++) {
        int d_poc = ref_pic_sets[ref_rps_idx].delta_poc_s1[j] + delta_rps;
        if (d_poc > 0 && use_delta[ref_num_negative_pics + j]) {
          out_ref_pic_set->delta_poc_s1[i] = d_poc;
          out_ref_pic_set->used_by_curr_pic_s1[i] =
              used_by_curr_pic[ref_num_negative_pics + j];
          i++;
        }
      }
      out_ref_pic_set->num_positive_pics = i;
    }
  } else {
    TRUE_OR_RETURN(br->ReadUE(&out_ref_pic_set->num_negative_pics));
    TRUE_OR_RETURN(out_ref_pic_set->num_negative_pics <= kMaxRefPicSetCount);
    TRUE_OR_RETURN(br->ReadUE(&out_ref_pic_set->num_positive_pics));
    TRUE_OR_RETURN(out_ref_pic_set->num_positive_pics <= kMaxRefPicSetCount);

    int prev_poc = 0;
    for (int i = 0; i < out_ref_pic_set->num_negative_pics; i++) {
      int delta_poc_s0_minus1;
      TRUE_OR_RETURN(br->ReadUE(&delta_poc_s0_minus1));
      out_ref_pic_set->delta_poc_s0[i] = prev_poc - (delta_poc_s0_minus1 + 1);
      prev_poc = out_ref_pic_set->delta_poc_s0[i];

      TRUE_OR_RETURN(br->ReadBool(&out_ref_pic_set->used_by_curr_pic_s0[i]));
    }

    prev_poc = 0;
    for (int i = 0; i < out_ref_pic_set->num_positive_pics; i++) {
      int delta_poc_s1_minus1;
      TRUE_OR_RETURN(br->ReadUE(&delta_poc_s1_minus1));
      out_ref_pic_set->delta_poc_s1[i] = prev_poc + delta_poc_s1_minus1 + 1;
      prev_poc = out_ref_pic_set->delta_poc_s1[i];

      TRUE_OR_RETURN(br->ReadBool(&out_ref_pic_set->used_by_curr_pic_s1[i]));
    }
  }

  out_ref_pic_set->num_delta_pocs =
      out_ref_pic_set->num_positive_pics + out_ref_pic_set->num_negative_pics;
  return kOk;
}

H265Parser::Result H265Parser::SkipReferencePictureListModification(
    const H265SliceHeader& slice_header,
    const H265Pps&,
    int num_pic_total_curr,
    H26xBitReader* br) {
  // Reads whole element but ignores it all.

  bool ref_pic_list_modification_flag_l0;
  TRUE_OR_RETURN(br->ReadBool(&ref_pic_list_modification_flag_l0));
  if (ref_pic_list_modification_flag_l0) {
    for (int i = 0; i <= slice_header.num_ref_idx_l0_active_minus1; i++) {
      TRUE_OR_RETURN(br->SkipBits(ceil(log2(num_pic_total_curr))));
    }
  }

  if (slice_header.slice_type == kBSlice) {
    bool ref_pic_list_modification_flag_l1;
    TRUE_OR_RETURN(br->ReadBool(&ref_pic_list_modification_flag_l1));
    if (ref_pic_list_modification_flag_l1) {
      for (int i = 0; i <= slice_header.num_ref_idx_l1_active_minus1; i++) {
        TRUE_OR_RETURN(br->SkipBits(ceil(log2(num_pic_total_curr))));
      }
    }
  }

  return kOk;
}

H265Parser::Result H265Parser::SkipPredictionWeightTablePart(
    int num_ref_idx_minus1,
    int chroma_array_type,
    H26xBitReader* br) {
  // Reads whole element, ignores it.
  int ignored;
  std::vector<bool> luma_weight_flag(num_ref_idx_minus1 + 1);
  std::vector<bool> chroma_weight_flag(num_ref_idx_minus1 + 1);

  for (int i = 0; i <= num_ref_idx_minus1; i++) {
    bool temp;
    TRUE_OR_RETURN(br->ReadBool(&temp));
    luma_weight_flag[i] = temp;
  }
  if (chroma_array_type != 0) {
    for (int i = 0; i <= num_ref_idx_minus1; i++) {
      bool temp;
      TRUE_OR_RETURN(br->ReadBool(&temp));
      chroma_weight_flag[i] = temp;
    }
  }
  for (int i = 0; i <= num_ref_idx_minus1; i++) {
    if (luma_weight_flag[i]) {
      TRUE_OR_RETURN(br->ReadSE(&ignored));  // delta_luma_weight_l#
      TRUE_OR_RETURN(br->ReadSE(&ignored));  // luma_offset_l#
    }
    if (chroma_weight_flag[i]) {
      for (int j = 0; j < 2; j++) {
        TRUE_OR_RETURN(br->ReadSE(&ignored));  // delta_chroma_weight_l#
        TRUE_OR_RETURN(br->ReadSE(&ignored));  // delta_chroma_offset_l#
      }
    }
  }

  return kOk;
}

H265Parser::Result H265Parser::SkipPredictionWeightTable(
    bool is_b_slice,
    const H265Sps& sps,
    const H265SliceHeader& slice_header,
    H26xBitReader* br) {
  // Reads whole element, ignores it.
  int ignored;
  int chroma_array_type = sps.GetChromaArrayType();

  TRUE_OR_RETURN(br->ReadUE(&ignored));  // luma_log2_weight_denom
  if (chroma_array_type != 0) {
    TRUE_OR_RETURN(br->ReadSE(&ignored));  // delta_chroma_log2_weight_denom
  }
  OK_OR_RETURN(SkipPredictionWeightTablePart(
      slice_header.num_ref_idx_l0_active_minus1, chroma_array_type, br));
  if (is_b_slice) {
    OK_OR_RETURN(SkipPredictionWeightTablePart(
        slice_header.num_ref_idx_l1_active_minus1, chroma_array_type, br));
  }

  return kOk;
}

H265Parser::Result H265Parser::ReadProfileTierLevel(
    bool profile_present,
    int max_num_sub_layers_minus1,
    H26xBitReader* br,
    int* prev_data,
    int* general_profile_tier_level_data) {
  // Reads whole element, ignores it.

  if (profile_present) {
    // 11 bytes of general_profile_tier flags:
    //   general_profile_space, general_tier_flag, general_profile_idc
    //   general_profile_compativility_flag
    //   general_progressive_source_flag
    //   general_interlaced_source_flag
    //   general_non_packed_constraint_flag
    //   general_frame_only_constraint_flag
    //   44-bits of other flags
    for (int i = 0; i < 11; i++)
      TRUE_OR_RETURN(br->ReadBits(8, &general_profile_tier_level_data[i]));
  } else if (prev_data != nullptr) {
    memcpy(general_profile_tier_level_data, prev_data,
           kGeneralProfileTierLevelBytes *
               sizeof(general_profile_tier_level_data[0]));
  }
  // general_level_idc
  TRUE_OR_RETURN(br->ReadBits(8, &general_profile_tier_level_data[11]));

  std::vector<bool> sub_layer_profile_present(max_num_sub_layers_minus1);
  std::vector<bool> sub_layer_level_present(max_num_sub_layers_minus1);
  for (int i = 0; i < max_num_sub_layers_minus1; i++) {
    bool profile, level;
    TRUE_OR_RETURN(br->ReadBool(&profile));
    TRUE_OR_RETURN(br->ReadBool(&level));
    sub_layer_profile_present[i] = profile;
    sub_layer_level_present[i] = level;
  }

  if (max_num_sub_layers_minus1 > 0) {
    for (int i = max_num_sub_layers_minus1; i < 8; i++)
      TRUE_OR_RETURN(br->SkipBits(2));  // reserved_zero_2bits
  }

  for (int i = 0; i < max_num_sub_layers_minus1; i++) {
    if (sub_layer_profile_present[i]) {
      // sub_layer_profile_space, sub_layer_tier_flag, sub_layer_profile_idc
      // sub_layer_profile_compatibility
      // sub_layer_reserved_zero_43bits
      // sub_layer_reserved_zero_bit
      TRUE_OR_RETURN(br->SkipBits(2 + 1 + 5 + 32 + 4 + 43 + 1));
    }
    if (sub_layer_level_present[i]) {
      TRUE_OR_RETURN(br->SkipBits(8));
    }
  }

  return kOk;
}

H265Parser::Result H265Parser::SkipScalingListData(H26xBitReader* br) {
  // Reads whole element, ignores it.
  int ignored;
  for (int size_id = 0; size_id < 4; size_id++) {
    for (int matrix_id = 0; matrix_id < 6;
         matrix_id += ((size_id == 3) ? 3 : 1)) {
      bool scaling_list_pred_mode;
      TRUE_OR_RETURN(br->ReadBool(&scaling_list_pred_mode));
      if (!scaling_list_pred_mode) {
        // scaling_list_pred_matrix_id_delta
        TRUE_OR_RETURN(br->ReadUE(&ignored));
      } else {
        int coefNum = std::min(64, (1 << (4 + (size_id << 1))));
        if (size_id > 1) {
          TRUE_OR_RETURN(br->ReadSE(&ignored));  // scaling_list_dc_coef_minus8
        }

        for (int i = 0; i < coefNum; i++) {
          TRUE_OR_RETURN(br->ReadSE(&ignored));  // scaling_list_delta_coef
        }
      }
    }
  }

  return kOk;
}

H265Parser::Result H265Parser::SkipHrdParameters(bool common_inf_present_flag,
                                                 int max_num_sub_layers_minus1,
                                                 H26xBitReader* br) {
  int ignored;
  bool nal_hdr_parameters_present_flag = false;
  bool vcl_hdr_parameters_present_flag = false;
  bool sub_pic_hdr_params_present_flag = false;
  if (common_inf_present_flag) {
    TRUE_OR_RETURN(br->ReadBool(&nal_hdr_parameters_present_flag));
    TRUE_OR_RETURN(br->ReadBool(&vcl_hdr_parameters_present_flag));
    if (nal_hdr_parameters_present_flag || vcl_hdr_parameters_present_flag) {
      TRUE_OR_RETURN(br->ReadBool(&sub_pic_hdr_params_present_flag));
      if (sub_pic_hdr_params_present_flag) {
        // tick_divisor_minus2, du_cpb_removal_delay_increment_length_minus1,
        // sub_pic_cpb_params_in_pic_timing_sei_flag
        // dpb_output_delay_du_length_minus1
        TRUE_OR_RETURN(br->SkipBits(8 + 5 + 1 + 5));
      }

      // bit_rate_scale, cpb_size_scale
      TRUE_OR_RETURN(br->SkipBits(4 + 4));
      if (sub_pic_hdr_params_present_flag)
        TRUE_OR_RETURN(br->SkipBits(4));  // cpb_size_du_scale

      // initial_cpb_removal_delay_length_minus1,
      // au_cpb_removal_delay_length_minus1, dpb_output_delay_length_minus1
      TRUE_OR_RETURN(br->SkipBits(5 + 5 + 5));
    }
  }

  for (int i = 0; i <= max_num_sub_layers_minus1; i++) {
    bool fixed_pic_rate_general_flag;
    bool fixed_pic_rate_within_cvs_flag = true;
    bool low_delay_hdr_flag = false;
    int cpb_cnt_minus1 = 0;
    TRUE_OR_RETURN(br->ReadBool(&fixed_pic_rate_general_flag));
    if (!fixed_pic_rate_general_flag)
      TRUE_OR_RETURN(br->ReadBool(&fixed_pic_rate_within_cvs_flag));
    if (fixed_pic_rate_within_cvs_flag)
      TRUE_OR_RETURN(br->ReadUE(&ignored));  // elemental_duration_ic_tc_minus1
    else
      TRUE_OR_RETURN(br->ReadBool(&low_delay_hdr_flag));
    if (!low_delay_hdr_flag)
      TRUE_OR_RETURN(br->ReadUE(&cpb_cnt_minus1));

    if (nal_hdr_parameters_present_flag) {
      OK_OR_RETURN(SkipSubLayerHrdParameters(
          cpb_cnt_minus1, sub_pic_hdr_params_present_flag, br));
    }
    if (vcl_hdr_parameters_present_flag) {
      OK_OR_RETURN(SkipSubLayerHrdParameters(
          cpb_cnt_minus1, sub_pic_hdr_params_present_flag, br));
    }
  }

  return kOk;
}

H265Parser::Result H265Parser::SkipSubLayerHrdParameters(
    int cpb_cnt_minus1,
    bool sub_pic_hdr_params_present_flag,
    H26xBitReader* br) {
  int ignored;
  for (int i = 0; i <= cpb_cnt_minus1; i++) {
    TRUE_OR_RETURN(br->ReadUE(&ignored));  // bit_rate_value_minus1
    TRUE_OR_RETURN(br->ReadUE(&ignored));  // cpb_size_value_minus1
    if (sub_pic_hdr_params_present_flag) {
      TRUE_OR_RETURN(br->ReadUE(&ignored));  // cpb_size_du_value_minus1
      TRUE_OR_RETURN(br->ReadUE(&ignored));  // bit_rate_du_value_minus1
    }

    TRUE_OR_RETURN(br->SkipBits(1));  // cbr_flag
  }

  return kOk;
}

H265Parser::Result H265Parser::ByteAlignment(H26xBitReader* br) {
  TRUE_OR_RETURN(br->SkipBits(1));
  TRUE_OR_RETURN(br->SkipBits(br->NumBitsLeft() % 8));
  return kOk;
}

}  // namespace media
}  // namespace shaka
