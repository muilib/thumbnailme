/***************************************************************************//**
*	@brief Thumbnail me 3.0
*	Thumbnail me is a user interface for Movie thumbnailer.
* 	Generate thumbnails from any movie is now easier !
*
*	@file	ThumbnailRunnable.cpp
*       @class  ThumbnailRunnable
*	Représente une instance runnable (thread).
*
*	@author Quentin Rousseau\n
*	@note   Copyright (C) 2011-2012 Quentin Rousseau\n
*               License: GNU General Public License version 2 (GPLv2) - http://www.gnu.org/licenses/gpl-2.0.html\n
*               Site web: www.thumbnailme.com\n
*               Email: quentin.rousseau@thumbnailme.com
*
*       @since      3.5
*	@version    3.5
*       @date       2011-2012
*******************************************************************************/

#include "ThumbnailRunnable.h"
#include "ThumbnailItem.h"
#include "MainWindow.h"
#include "DockInputOutput.h"
#include "DockStyles.h"
#include "DockConf.h"
#include "DockTimeLine.h"
#include "defines.h"

ThumbnailRunnable::ThumbnailRunnable(MainWindow *main_window, ThumbnailItem *item, QString suffix, ThumbnailEngine::Mode mode)
{
    this->main_window = main_window;
    this->item = item;
    this->suffix = suffix;
    this->modeConversion = mode;
}

void ThumbnailRunnable::run()
{
    emit started(this->item);

    QColor c;

    argv0Array = this->item->getFilePath().toString().toLocal8Bit();
    parameters.gb_argv0 = argv0Array.data();

    parameters.gb_a_ratio = GB_A_RATIO;
    parameters.gb_C_cut = GB_C_CUT;
    parameters.gb_h_height = 0;
    parameters.gb_I_individual = GB_I_INDIVIDUAL;
    parameters.gb_z_seek    = GB_Z_SEEK;
    parameters.gb_Z_nonseek = GB_Z_NONSEEK;
    parameters.gb_s_step    = GB_S_STEP;
    parameters.gb_O_format = main_window->mpDockConf->getFormatFileCode();
    parameters.gb_r_row     = main_window->mpDockConf->getRows();
    parameters.gb_c_column  = main_window->mpDockConf->getColumns();
    parameters.gb_w_width   = main_window->mpDockConf->getWidth();
    parameters.gb_g_gap     = main_window->mpDockConf->getGap();
    parameters.gb_j_quality = main_window->mpDockConf->getQuality();
    parameters.gb_b_blank   = main_window->mpDockConf->getBlankSkip();
    parameters.gb_D_edge    = GB_D_EDGE;
    parameters.gb_L_info_location = main_window->mpDockStyles->getInfoTextLocation();
    parameters.gb_L_time_location = main_window->mpDockStyles->getTimeStampLocation();
    c = main_window->mpDockStyles->getColorBackground();
    parameters.gb_k_bcolor   = (rgb_color) {c.red(), c.green(), c.blue()};

    f_fontnameArray = main_window->mpDockStyles->getFontInfoText(0).toLocal8Bit();
    parameters.gb_f_fontname = f_fontnameArray.data();

    c = main_window->mpDockStyles->getColorInfoText();
    parameters.gb_F_info_color = (rgb_color) {c.red(), c.green(), c.blue()};
    parameters.gb_F_info_font_size = main_window->mpDockStyles->getSizeInfoText();

    if(main_window->mpDockStyles->isTimeStampChecked())
    {
         parameters.gb_t_timestamp = 1;
         F_ts_fontnameArray = main_window->mpDockStyles->getFontTimeStamp(2).toLocal8Bit();
         parameters.gb_F_ts_fontname  = F_ts_fontnameArray.data();
         c = main_window->mpDockStyles->getColorTimeStamp();
         parameters.gb_F_ts_color     = (rgb_color) {c.red(), c.green(), c.blue()};
         c = main_window->mpDockStyles->getColorShadow();
         parameters.gb_F_ts_shadow    = (rgb_color) {c.red(), c.green(), c.blue()};
         parameters.gb_F_ts_font_size = main_window->mpDockStyles->getSizeTimeStamp();
    }
    else parameters.gb_t_timestamp = 0;

    if(this->modeConversion == ThumbnailEngine::SIMPLEMOD && !main_window->mpDockInputOutput->isSameSourceChecked())
    {
        O_outdir = main_window->mpDockInputOutput->getPathOutput().toLocal8Bit();
        parameters.gb_O_outdir = O_outdir.data();

        if(!this->suffix.isEmpty())
        {
           o_suffixArray = QString(this->suffix + "." + this->main_window->mpDockConf->getFormatFile()).toLocal8Bit();
           parameters.gb_o_suffix = o_suffixArray.data();
        }
        else
        {
            o_suffixArray = QString("." + this->main_window->mpDockConf->getFormatFile()).toLocal8Bit();
            parameters.gb_o_suffix = o_suffixArray.data();
        }
    }
    else if (this->modeConversion == ThumbnailEngine::SIMPLEMOD && main_window->mpDockInputOutput->isSameSourceChecked())
    {
        O_outdir = QDir::toNativeSeparators(QFileInfo(item->getFilePath().toString()).canonicalPath()).toLocal8Bit();
        parameters.gb_O_outdir = O_outdir.data();

        if(!this->suffix.isEmpty())
        {
           o_suffixArray = QString(this->suffix + "." + this->main_window->mpDockConf->getFormatFile()).toLocal8Bit();
           parameters.gb_o_suffix = o_suffixArray.data();
        }
        else
        {
            o_suffixArray = QString("." + this->main_window->mpDockConf->getFormatFile()).toLocal8Bit();
            parameters.gb_o_suffix = o_suffixArray.data();
        }
    }
    else
    {
        O_outdir = QDir::tempPath().toLocal8Bit();
        parameters.gb_O_outdir = O_outdir.data();

        o_suffixArray = DEFAULT_TMP_EXTENSION.toLocal8Bit();
        parameters.gb_o_suffix = o_suffixArray.data();
    }

    if(main_window->mpDockStyles->isInfoTextChecked())
         parameters.gb_i_info = 1;
    else parameters.gb_i_info = 0;

    if(!main_window->mpDockStyles->getTitleEdit().isEmpty())
    {
        T_textArray = main_window->mpDockStyles->getTitleEdit().toLocal8Bit();
        parameters.gb_T_text = T_textArray.data();
    }

    if(!this->item->isReadable() && !!this->item->getLowerTime().isNull() && !this->item->getUpperTime().isNull())
    {
        parameters.gb_B_begin = this->item->getBeginOmmitSecs();
        parameters.gb_E_end = this->item->getEndOmmitSecs();
    }

    process_file();

    this->item->logs = QString::fromUtf8(logs);

    emit finished(this->item);
}
