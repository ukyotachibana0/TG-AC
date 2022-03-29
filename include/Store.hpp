/**
 * create time : 2020-10-14
 * Store header
 */
#pragma once

#include <set>
#include "headers.hpp"
#include "types.hpp"
#include "BufferWriter.hpp"

namespace gl {
namespace fastsgg {

class Store
{
private:
    BufferWriter *bw;
    BufferWriter *bw_csroff;
    
    std::string base_filename;  // base filename
    EnumStoreFormat format;
    int buffer_size;

    std::string csf_off_suffix; // CSROFF

    int_t file_lines_upper;     // #Lines upper in a file
    int_t current_file_lines;   // #Lines in this file
    int_t current_file_no;      // #FileNo of current file
    int_t offset;               // CSR offset

public:
    Store() {
        bw = bw_csroff = nullptr;

        base_filename = "part_";
        format = EnumStoreFormat::TSV;
        buffer_size = 1024*1024*32;

        csf_off_suffix = "_CSROFF_";

        file_lines_upper = 1000000;
        current_file_lines = 0;
        current_file_no = 0;
        offset = 0;

        nextFile();
    }

    Store(std::string& filename, EnumStoreFormat _format,  int _buffer_size=1024*1024*32) {
        bw = bw_csroff = nullptr;

        base_filename = filename;
        format = _format;
        buffer_size = _buffer_size;

        csf_off_suffix = "_CSROFF_";

        file_lines_upper = 1000000;
        current_file_lines = 0;
        current_file_no = 1;
        offset = 0;

        nextFile();
    }

    void nextFile() {
        close();
        std::string file_no = Utility::strSeqNumber(current_file_no);
        std::string fn = base_filename + file_no;
        bw = new BufferWriter(fn.c_str(), buffer_size);
        if (format == EnumStoreFormat::CSR) {
            offset = 0;
            std::string csr_off_filename = base_filename + csf_off_suffix + file_no;
            bw_csroff = new BufferWriter(csr_off_filename.c_str(), buffer_size);
        }
        // update
        current_file_lines = 0;
        current_file_no ++;
    }

    void writeTSVLine(int_t src, int_t tgt, std::string attach="", bool reverse=false) {
#ifdef PATCH_VPP
        // PATCH
        std::string a_src = std::to_string(src);
        std::string b_tgt = std::to_string(tgt);
        if (attach.find("Page") != std::string::npos) {
            a_src = "vp_" + a_src;
            b_tgt = "p_" + b_tgt;
        } else {
            a_src = "vp_" + a_src;
            b_tgt = "vp_" + b_tgt;
        }
        if (reverse) {
            bw->write(b_tgt);
            bw->tab();
            bw->write(a_src);
        } else {
            bw->write(a_src);
            bw->tab();
            bw->write(b_tgt);
        }
#else
        if (!reverse) {
            bw->write(src);
            bw->tab();
            bw->write(tgt);
        } else {
            bw->write(tgt);
            bw->tab();
            bw->write(src);
        }
#endif
        // END PATCH
        if (!attach.empty()) {
            bw->tab();
            bw->write(attach);
        }
        bw->newline();
        // update #Lines
        current_file_lines ++;
        if (current_file_lines == file_lines_upper) {
            nextFile();
        }
    }

    void writeLine(int_t i, std::unordered_set<int_t>& adj) {
        if (adj.empty())
            return;
        if (format == EnumStoreFormat::TSV) { // TSV
            for (auto n : adj) {
                bw->write(i);
                bw->tab();
                bw->write(n);
                bw->newline();
                // update #Lines
                current_file_lines ++;
                if (current_file_lines == file_lines_upper) {
                    nextFile();
                }
            }
        } else if (format == EnumStoreFormat::ADJ) { // ADJ
            bw->write(i);
            for (auto n : adj) {
                bw->space();
                bw->write(n);
            }
            bw->newline();
            // update #Lines
            current_file_lines ++;
            if (current_file_lines == file_lines_upper) {
                nextFile();
            }
        } else { //  CSR
            for (auto n : adj) {
                bw->write(n);
                bw->tab();
            }
            // offset
            bw_csroff->write(offset);
            bw_csroff->tab();
            offset += adj.size();
        }
    }

    void writeLine(int_t i, std::set<std::pair<int_t, int_t>>& adj) {
        if (adj.empty()) return;

        int_t n = adj.size();
        // TODO: consider different formats
        for (auto p : adj) {    // TSV
            bw->write(i);
            bw->tab();
            bw->write(p.first);
            bw->tab();
            bw->write(p.second);
            bw->newline();
            current_file_lines ++;
            if (current_file_lines == file_lines_upper) {
                nextFile();
            }
        }
    }

    void flush() {
        if (bw) {
            bw->flush();
        }
        if (bw_csroff) {
            bw_csroff->flush();
        }
    }

    void close() {
        flush();
        if (bw) {
            bw->close();
            bw = nullptr;
        }
        if (bw_csroff) {
            bw_csroff->close();
            bw_csroff = nullptr;
        }
    }

    ~Store() {
        close();
    }
}; //! class Store

} //! namespace fastsgg
} //! namespace gl