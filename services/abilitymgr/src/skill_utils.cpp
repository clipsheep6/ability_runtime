/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "skill_utils.h"

#include <regex>
#include <unistd.h>
#include "string_ex.h"
#include "parameters.h"

namespace OHOS {
namespace AAFwk {

const std::string SCHEME_SEPARATOR = "://";
const std::string PORT_SEPARATOR = ":";
const std::string PATH_SEPARATOR = "/";
const std::string PARAM_SEPARATOR = "?";
const char WILDCARD = '*';
const std::string TYPE_WILDCARD = "*/*";
const char* TYPE_ONLY_MATCH_WILDCARD = "reserved/wildcard";
const char* ACTION_HOME = "action.system.home";
const char* WANT_ACTION_HOME = "ohos.want.action.home";

std::multimap<std::string, std::string> SkillUtils::mimeTypeMap_ = {
    {"323", "text/h323"},
    {"3g2", "video/3gpp2"},
    {"3gp", "video/3gpp"},
    {"3gpp", "audio/3gpp"},
    {"3gpp", "video/3gpp"},
    {"3gpp2", "video/3gpp2"},
    {"VOB", "video/mpeg"},
    {"aac", "audio/aac"},
    {"aac", "audio/aac-adts"},
    {"abw", "application/x-abiword"},
    {"aif", "audio/x-aiff"},
    {"aifc", "audio/x-aiff"},
    {"aiff", "audio/x-aiff"},
    {"amr", "audio/amr"},
    {"asc", "text/plain"},
    {"asf", "video/x-ms-asf"},
    {"asx", "video/x-ms-asf"},
    {"avi", "video/avi"},
    {"awb", "audio/amr-wb"},
    {"bcpio", "application/x-bcpio"},
    {"bib", "text/x-bibtex"},
    {"bmp", "image/bmp"},
    {"bmp", "image/x-ms-bmp"},
    {"boo", "text/x-boo"},
    {"book", "application/x-maker"},
    {"c", "text/x-csrc"},
    {"c++", "text/x-c++src"},
    {"cc", "text/x-c++src"},
    {"cdf", "application/x-cdf"},
    {"cdr", "image/x-coreldraw"},
    {"cdt", "image/x-coreldrawtemplate"},
    {"cdy", "application/vnd.cinderella"},
    {"cer", "application/pkix-cert"},
    {"chrt", "application/x-kchart"},
    {"cls", "text/x-tex"},
    {"cod", "application/vnd.rim.cod"},
    {"cpio", "application/x-cpio"},
    {"cpp", "text/x-c++src"},
    {"cpt", "image/x-corelphotopaint"},
    {"crl", "application/x-pkcs7-crl"},
    {"crt", "application/x-x509-ca-cert"},
    {"crt", "application/x-x509-server-cert"},
    {"crt", "application/x-x509-user-cert"},
    {"csh", "text/x-csh"},
    {"css", "text/css"},
    {"csv", "text/comma-separated-values"},
    {"cur", "image/ico"},
    {"cxx", "text/x-c++src"},
    {"d", "text/x-dsrc"},
    {"dcr", "application/x-director"},
    {"deb", "application/x-debian-package"},
    {"dif", "video/dv"},
    {"diff", "text/plain"},
    {"dir", "application/x-director"},
    {"djv", "image/vnd.djvu"},
    {"djvu", "image/vnd.djvu"},
    {"dl", "video/dl"},
    {"dmg", "application/x-apple-diskimage"},
    {"dms", "application/x-dms"},
    {"doc", "application/msword"},
    {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {"dot", "application/msword"},
    {"dotx", "application/vnd.openxmlformats-officedocument.wordprocessingml.template"},
    {"dv", "video/dv"},
    {"dvi", "application/x-dvi"},
    {"dxr", "application/x-director"},
    {"etx", "text/x-setext"},
    {"ez", "application/andrew-inset"},
    {"fb", "application/x-maker"},
    {"fbdoc", "application/x-maker"},
    {"fig", "application/x-xfig"},
    {"flac", "application/x-flac"},
    {"flac", "audio/flac"},
    {"fli", "video/fli"},
    {"frame", "application/x-maker"},
    {"frm", "application/x-maker"},
    {"gcd", "text/x-pcs-gcd"},
    {"gcf", "application/x-graphing-calculator"},
    {"gif", "image/gif"},
    {"gnumeric", "application/x-gnumeric"},
    {"gsf", "application/x-font"},
    {"gsm", "audio/x-gsm"},
    {"gtar", "application/x-gtar"},
    {"h", "text/x-chdr"},
    {"h++", "text/x-c++hdr"},
    {"hdf", "application/x-hdf"},
    {"hh", "text/x-c++hdr"},
    {"hpp", "text/x-c++hdr"},
    {"hqx", "application/mac-binhex40"},
    {"hs", "text/x-haskell"},
    {"hta", "application/hta"},
    {"htc", "text/x-component"},
    {"htm", "text/html"},
    {"html", "text/html"},
    {"hxx", "text/x-c++hdr"},
    {"ica", "application/x-ica"},
    {"ice", "x-conference/x-cooltalk"},
    {"ico", "image/ico"},
    {"ico", "image/x-icon"},
    {"ics", "text/calendar"},
    {"icz", "text/calendar"},
    {"ief", "image/ief"},
    {"iges", "model/iges"},
    {"igs", "model/iges"},
    {"iii", "application/x-iphone"},
    {"imy", "audio/imelody"},
    {"ins", "application/x-internet-signup"},
    {"iso", "application/x-iso9660-image"},
    {"isp", "application/x-internet-signup"},
    {"java", "text/x-java"},
    {"jmz", "application/x-jmol"},
    {"jng", "image/x-jng"},
    {"jpe", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"jpg", "image/jpeg"},
    {"kar", "audio/midi"},
    {"key", "application/pgp-keys"},
    {"kil", "application/x-killustrator"},
    {"kpr", "application/x-kpresenter"},
    {"kpt", "application/x-kpresenter"},
    {"ksp", "application/x-kspread"},
    {"kwd", "application/x-kword"},
    {"kwt", "application/x-kword"},
    {"latex", "application/x-latex"},
    {"lha", "application/x-lha"},
    {"lhs", "text/x-literate-haskell"},
    {"lsf", "video/x-la-asf"},
    {"lsx", "video/x-la-asf"},
    {"ltx", "text/x-tex"},
    {"lzh", "application/x-lzh"},
    {"lzx", "application/x-lzx"},
    {"m3u", "audio/mpegurl"},
    {"m3u", "audio/x-mpegurl"},
    {"m4a", "audio/mpeg"},
    {"m4v", "video/m4v"},
    {"maker", "application/x-maker"},
    {"man", "application/x-troff-man"},
    {"mdb", "application/msaccess"},
    {"mesh", "model/mesh"},
    {"mid", "audio/midi"},
    {"midi", "audio/midi"},
    {"mif", "application/x-mif"},
    {"mka", "audio/x-matroska"},
    {"mkv", "video/x-matroska"},
    {"mm", "application/x-freemind"},
    {"mmf", "application/vnd.smaf"},
    {"mml", "text/mathml"},
    {"mng", "video/x-mng"},
    {"moc", "text/x-moc"},
    {"mov", "video/quicktime"},
    {"movie", "video/x-sgi-movie"},
    {"mp2", "audio/mpeg"},
    {"mp3", "audio/mpeg"},
    {"mp4", "video/mp4"},
    {"mpe", "video/mpeg"},
    {"mpeg", "video/mpeg"},
    {"mpega", "audio/mpeg"},
    {"mpg", "video/mpeg"},
    {"mpga", "audio/mpeg"},
    {"msh", "model/mesh"},
    {"msi", "application/x-msi"},
    {"mxmf", "audio/mobile-xmf"},
    {"mxu", "video/vnd.mpegurl"},
    {"nb", "application/mathematica"},
    {"nwc", "application/x-nwc"},
    {"o", "application/x-object"},
    {"oda", "application/oda"},
    {"odb", "application/vnd.oasis.opendocument.database"},
    {"odf", "application/vnd.oasis.opendocument.formula"},
    {"odg", "application/vnd.oasis.opendocument.graphics"},
    {"odi", "application/vnd.oasis.opendocument.image"},
    {"ods", "application/vnd.oasis.opendocument.spreadsheet"},
    {"odt", "application/vnd.oasis.opendocument.text"},
    {"oga", "application/ogg"},
    {"ogg", "application/ogg"},
    {"ota", "audio/midi"},
    {"otg", "application/vnd.oasis.opendocument.graphics-template"},
    {"oth", "application/vnd.oasis.opendocument.text-web"},
    {"ots", "application/vnd.oasis.opendocument.spreadsheet-template"},
    {"ott", "application/vnd.oasis.opendocument.text-template"},
    {"oza", "application/x-oz-application"},
    {"p", "text/x-pascal"},
    {"p12", "application/x-pkcs12"},
    {"p7r", "application/x-pkcs7-certreqresp"},
    {"pac", "application/x-ns-proxy-autoconfig"},
    {"pas", "text/x-pascal"},
    {"pat", "image/x-coreldrawpattern"},
    {"pbm", "image/x-portable-bitmap"},
    {"pcf", "application/x-font"},
    {"pcf.z", "application/x-font"},
    {"pcx", "image/pcx"},
    {"pdf", "application/pdf"},
    {"pem", "application/x-pem-file"},
    {"pfa", "application/x-font"},
    {"pfb", "application/x-font"},
    {"pfx", "application/x-pkcs12"},
    {"pgm", "image/x-portable-graymap"},
    {"pgn", "application/x-chess-pgn"},
    {"pgp", "application/pgp-signature"},
    {"phps", "text/text"},
    {"pls", "audio/x-scpls"},
    {"png", "image/png"},
    {"pnm", "image/x-portable-anymap"},
    {"po", "text/plain"},
    {"pot", "application/vnd.ms-powerpoint"},
    {"potx", "application/vnd.openxmlformats-officedocument.presentationml.template"},
    {"ppm", "image/x-portable-pixmap"},
    {"pps", "application/vnd.ms-powerpoint"},
    {"ppsx", "application/vnd.openxmlformats-officedocument.presentationml.slideshow"},
    {"ppt", "application/vnd.ms-powerpoint"},
    {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {"prf", "application/pics-rules"},
    {"psd", "image/x-photoshop"},
    {"qt", "video/quicktime"},
    {"qtl", "application/x-quicktimeplayer"},
    {"ra", "audio/x-pn-realaudio"},
    {"ra", "audio/x-realaudio"},
    {"ram", "audio/x-pn-realaudio"},
    {"rar", "application/rar"},
    {"ras", "image/x-cmu-raster"},
    {"rdf", "application/rdf+xml"},
    {"rgb", "image/x-rgb"},
    {"rm", "audio/x-pn-realaudio"},
    {"roff", "application/x-troff"},
    {"rss", "application/rss+xml"},
    {"rtf", "text/rtf"},
    {"rtttl", "audio/midi"},
    {"rtx", "text/richtext"},
    {"sd2", "audio/x-sd2"},
    {"sda", "application/vnd.stardivision.draw"},
    {"sdc", "application/vnd.stardivision.calc"},
    {"sdd", "application/vnd.stardivision.impress"},
    {"sdp", "application/vnd.stardivision.impress"},
    {"sdw", "application/vnd.stardivision.writer"},
    {"sgf", "application/x-go-sgf"},
    {"sgl", "application/vnd.stardivision.writer-global"},
    {"shar", "application/x-shar"},
    {"sid", "audio/prs.sid"},
    {"silo", "model/mesh"},
    {"sisx", "x-epoc/x-sisx-app"},
    {"sit", "application/x-stuffit"},
    {"skd", "application/x-koan"},
    {"skm", "application/x-koan"},
    {"skp", "application/x-koan"},
    {"skt", "application/x-koan"},
    {"smf", "application/vnd.stardivision.math"},
    {"snd", "audio/basic"},
    {"spl", "application/futuresplash"},
    {"spl", "application/x-futuresplash"},
    {"src", "application/x-wais-source"},
    {"stc", "application/vnd.sun.xml.calc.template"},
    {"std", "application/vnd.sun.xml.draw.template"},
    {"sti", "application/vnd.sun.xml.impress.template"},
    {"stl", "application/vnd.ms-pki.stl"},
    {"stw", "application/vnd.sun.xml.writer.template"},
    {"sty", "text/x-tex"},
    {"sv4cpio", "application/x-sv4cpio"},
    {"sv4crc", "application/x-sv4crc"},
    {"svg", "image/svg+xml"},
    {"svgz", "image/svg+xml"},
    {"swf", "application/x-shockwave-flash"},
    {"sxc", "application/vnd.sun.xml.calc"},
    {"sxd", "application/vnd.sun.xml.draw"},
    {"sxg", "application/vnd.sun.xml.writer.global"},
    {"sxi", "application/vnd.sun.xml.impress"},
    {"sxm", "application/vnd.sun.xml.math"},
    {"sxw", "application/vnd.sun.xml.writer"},
    {"t", "application/x-troff"},
    {"tar", "application/x-tar"},
    {"taz", "application/x-gtar"},
    {"tcl", "text/x-tcl"},
    {"tex", "text/x-tex"},
    {"texi", "application/x-texinfo"},
    {"texinfo", "application/x-texinfo"},
    {"text", "text/plain"},
    {"tgz", "application/x-gtar"},
    {"tif", "image/tiff"},
    {"tiff", "image/tiff"},
    {"torrent", "application/x-bittorrent"},
    {"ts", "video/mp2ts"},
    {"tsp", "application/dsptype"},
    {"tsv", "text/tab-separated-values"},
    {"txt", "text/plain"},
    {"udeb", "application/x-debian-package"},
    {"uls", "text/iuls"},
    {"ustar", "application/x-ustar"},
    {"vcd", "application/x-cdlink"},
    {"vcf", "text/x-vcard"},
    {"vcs", "text/x-vcalendar"},
    {"vor", "application/vnd.stardivision.writer"},
    {"vsd", "application/vnd.visio"},
    {"wad", "application/x-doom"},
    {"wav", "audio/x-wav"},
    {"wax", "audio/x-ms-wax"},
    {"wbmp", "image/vnd.wap.wbmp"},
    {"webarchive", "application/x-webarchive"},
    {"webarchivexml", "application/x-webarchive-xml"},
    {"webm", "video/webm"},
    {"webp", "image/webp"},
    {"wm", "video/x-ms-wm"},
    {"wma", "audio/x-ms-wma"},
    {"wmd", "application/x-ms-wmd"},
    {"wmv", "video/x-ms-wmv"},
    {"wmx", "video/x-ms-wmx"},
    {"wmz", "application/x-ms-wmz"},
    {"wrf", "video/x-webex"},
    {"wvx", "video/x-ms-wvx"},
    {"wz", "application/x-wingz"},
    {"xbm", "image/x-xbitmap"},
    {"xcf", "application/x-xcf"},
    {"xhtml", "application/xhtml+xml"},
    {"xls", "application/vnd.ms-excel"},
    {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {"xlt", "application/vnd.ms-excel"},
    {"xltx", "application/vnd.openxmlformats-officedocument.spreadsheetml.template"},
    {"xmf", "audio/midi"},
    {"xml", "text/xml"},
    {"xpm", "image/x-xpixmap"},
    {"xwd", "image/x-xwindowdump"},
    {"zip", "application/zip"},
    {"eddx", "application-x-eddx"},
    {"caj", "application/caj"},
    {"kdh", "application/kdh"},
    {"nh", "application/nh"},
    {"teb", "application/teb"},
    {"epub", "application/epub+zip"},
    {"xlam", "application/vnd.ms-excel.addin.macroEnabled.12"},
    {"xlsb", "application/vnd.ms-excel.sheet.binary.macroEnabled.12"},
    {"xlsm", "application/vnd.ms-excel.sheet.macroEnabled.12"},
    {"xltm", "application/vnd.ms-excel.template.macroEnabled.12"},
    {"ppam", "application/vnd.ms-powerpoint.addin.macroEnabled.12"},
    {"pptm", "application/vnd.ms-powerpoint.presentation.macroEnabled.12"},
    {"ppsm", "application/vnd.ms-powerpoint.slideshow.macroEnabled.12"},
    {"potm", "application/vnd.ms-powerpoint.template.macroEnabled.12"},
    {"docm", "application/vnd.ms-word.document.macroEnabled.12"},
    {"dotm", "application/vnd.ms-word.template.macroEnabled.12"},
    {"odc", "application/vnd.oasis.opendocument.chart"},
    {"odp", "application/vnd.oasis.opendocument.presentation"},
    {"odm", "application/vnd.oasis.opendocument.text-master"},
    {"odw", "application/vnd.oasis.opendocument.text-web"},
    {"otp", "application/vnd.oasis.opendocument.presentation-template"},
    {"m3u8", "application/vnd.apple.mpegurl"},
    {"sds", "application/vnd.stardivision.chart"},
    {"sdm", "application/vnd.stardivision.mail"},
    {"pub", "application/x-mspublisher"},
    {"wpl", "application/vnd.ms-wpl"},
};


SkillUtils::SkillUtils() {}
SkillUtils::~SkillUtils() {}

void SkillUtils::SetActionsAndEntitiesAndUri(const std::vector<std::string> &tempActions,
    const std::vector<std::string> &tempEntities, const std::vector<OHOS::AppExecFwk::SkillUriForAbility> &tempUris)
{
    actions = tempActions;
    entities = tempEntities;
    for(auto skillUriForAbility : tempUris)
    {
        SkillUri skillUri;
        skillUri.scheme = skillUriForAbility.scheme;
        skillUri.host = skillUriForAbility.host;
        skillUri.port = skillUriForAbility.port;
        skillUri.path = skillUriForAbility.path;
        skillUri.pathStartWith = skillUriForAbility.pathStartWith;
        skillUri.pathRegex = skillUriForAbility.pathRegex;
        skillUri.type = skillUriForAbility.type;
        skillUri.utd = skillUriForAbility.utd;
        skillUri.maxFileSupported = skillUriForAbility.maxFileSupported;
        skillUri.linkFeature = skillUriForAbility.linkFeature;

        uris.push_back(skillUri);
    }

}

bool SkillUtils::Match(const OHOS::AAFwk::Want &want) const
{
    if (!MatchActionAndEntities(want)) {
        APP_LOGD("Action or entities does not match");
        return false;
    }
    std::vector<std::string> vecTypes = want.GetStringArrayParam(OHOS::AAFwk::Want::PARAM_ABILITY_URITYPES);
    if (vecTypes.size() > 0) {
        for (std::string strType : vecTypes) {
            if (MatchUriAndType(want.GetUriString(), strType)) {
                APP_LOGD("type %{public}s, Is Matched", strType.c_str());
                return true;
            }
        }
        return false;
    }
    bool matchUriAndType = MatchUriAndType(want.GetUriString(), want.GetType());
    if (!matchUriAndType) {
        APP_LOGD("Uri or Type does not match");
        return false;
    }
    return true;
}

bool SkillUtils::MatchType(const std::string &type, const std::string &skillUriType) const
{
    // type is not empty
    if (skillUriType.empty()) {
        return false;
    }
    // only match */*
    if (type == TYPE_ONLY_MATCH_WILDCARD) {
        return skillUriType == TYPE_WILDCARD;
    }
    if (type == TYPE_WILDCARD || skillUriType == TYPE_WILDCARD) {
        // param is */* or config is */*
        return true;
    }
    bool paramTypeRegex = type.back() == WILDCARD;
    if (paramTypeRegex) {
        // param is string/*
        std::string prefix = type.substr(0, type.length() - 1);
        return skillUriType.find(prefix) == 0;
    }
    bool typeRegex = skillUriType.back() == WILDCARD;
    if (typeRegex) {
        // config is string/*
        std::string prefix = skillUriType.substr(0, skillUriType.length() - 1);
        return type.find(prefix) == 0;
    } else {
        return type == skillUriType;
    }
}

bool SkillUtils::MatchActionAndEntities(const OHOS::AAFwk::Want &want) const
{
    bool matchAction = MatchAction(want.GetAction());
    if (!matchAction) {
        APP_LOGD("Action does not match");
        return false;
    }
    bool matchEntities = MatchEntities(want.GetEntities());
    if (!matchEntities) {
        APP_LOGD("Entities does not match");
        return false;
    }
    return true;
}

bool SkillUtils::MatchAction(const std::string &action) const
{
    // config actions empty, no match
    if (actions.empty()) {
        return false;
    }
    // config actions not empty, param empty, match
    if (action.empty()) {
        return true;
    }
    auto actionMatcher = [action] (const std::string &configAction) {
        if (action == configAction) {
            return true;
        }
        if (action == ACTION_HOME && configAction == WANT_ACTION_HOME) {
            return true;
        }
        if (action == WANT_ACTION_HOME && configAction == ACTION_HOME) {
            return true;
        }
        return false;
    };
    // config actions not empty, param not empty, if config actions contains param action, match
    return std::find_if(actions.cbegin(), actions.cend(), actionMatcher) != actions.cend();
}

bool SkillUtils::MatchEntities(const std::vector<std::string> &paramEntities) const
{
    // param entities empty, match
    if (paramEntities.empty()) {
        return true;
    }
    // config entities empty, param entities not empty, not match
    if (entities.empty()) {
        return false;
    }
    // config entities not empty, param entities not empty, if every param entity in config entities, match
    std::vector<std::string>::size_type size = paramEntities.size();
    for (std::vector<std::string>::size_type i = 0; i < size; i++) {
        bool ret = std::find(entities.cbegin(), entities.cend(), paramEntities[i]) == entities.cend();
        if (ret) {
            return false;
        }
    }
    return true;
}

bool SkillUtils::MatchUriAndType(const std::string &uriString, const std::string &type) const
{
    if (uriString.empty() && type.empty()) {
        // case1 : param uri empty, param type empty
        if (uris.empty()) {
            return true;
        }
        for (const SkillUri &skillUri : uris) {
            if (skillUri.scheme.empty() && skillUri.type.empty()) {
                return true;
            }
        }
        return false;
    }
    if (uris.empty()) {
        return false;
    }
    if (!uriString.empty() && type.empty()) {
        // case2 : param uri not empty, param type empty
        for (const SkillUri &skillUri : uris) {
            if (MatchUri(uriString, skillUri) && skillUri.type.empty()) {
                return true;
            }
        }
        // if uri is a file path, match type by the suffix
        return MatchMimeType(uriString);
    } else if (uriString.empty() && !type.empty()) {
        // case3 : param uri empty, param type not empty
        for (const SkillUri &skillUri : uris) {
            if (skillUri.scheme.empty() && MatchType(type, skillUri.type)) {
                return true;
            }
        }
        return false;
    } else {
        // case4 : param uri not empty, param type not empty
        for (const SkillUri &skillUri : uris) {
            if (MatchUri(uriString, skillUri) && MatchType(type, skillUri.type)) {
                return true;
            }
        }
        return false;
    }
}

bool SkillUtils::MatchUri(const std::string &uriString, const SkillUri &skillUri) const
{
    if (skillUri.scheme.empty()) {
        return uriString.empty();
    }
    if (skillUri.host.empty()) {
        // config uri is : scheme
        // belows are param uri matched conditions:
        // 1.scheme
        // 2.scheme:
        // 3.scheme:/
        // 4.scheme://
        return uriString == skillUri.scheme || StartsWith(uriString, skillUri.scheme + PORT_SEPARATOR);
    }
    std::string optParamUri = GetOptParamUri(uriString);
    std::string skillUriString;
    skillUriString.append(skillUri.scheme).append(SCHEME_SEPARATOR).append(skillUri.host);
    if (!skillUri.port.empty()) {
        skillUriString.append(PORT_SEPARATOR).append(skillUri.port);
    }
    if (skillUri.path.empty() && skillUri.pathStartWith.empty() && skillUri.pathRegex.empty()) {
        // with port, config uri is : scheme://host:port
        // belows are param uri matched conditions:
        // 1.scheme://host:port
        // 2.scheme://host:port/path

        // without port, config uri is : scheme://host
        // belows are param uri matched conditions:
        // 1.scheme://host
        // 2.scheme://host/path
        // 3.scheme://host:port     scheme://host:port/path
        bool ret = (optParamUri == skillUriString || StartsWith(optParamUri, skillUriString + PATH_SEPARATOR));
        if (skillUri.port.empty()) {
            ret = ret || StartsWith(optParamUri, skillUriString + PORT_SEPARATOR);
        }
        return ret;
    }
    skillUriString.append(PATH_SEPARATOR);
    // if one of path, pathStartWith, pathRegex match, then match
    if (!skillUri.path.empty()) {
        // path match
        std::string pathUri(skillUriString);
        pathUri.append(skillUri.path);
        if (optParamUri == pathUri) {
            return true;
        }
    }
    if (!skillUri.pathStartWith.empty()) {
        // pathStartWith match
        std::string pathStartWithUri(skillUriString);
        pathStartWithUri.append(skillUri.pathStartWith);
        if (StartsWith(optParamUri, pathStartWithUri)) {
            return true;
        }
    }
    if (!skillUri.pathRegex.empty()) {
        // pathRegex match
        std::string pathRegexUri(skillUriString);
        pathRegexUri.append(skillUri.pathRegex);
        try {
            std::regex regex(pathRegexUri);
            if (regex_match(optParamUri, regex)) {
                return true;
            }
        } catch(...) {
            APP_LOGE("regex error");
        }
    }
    return false;
}

bool SkillUtils::MatchMimeType(const std::string & uriString) const
{
    std::vector<std::string> mimeTypes;
    bool ret = GetMimeTypeByUri(uriString, mimeTypes);
    if (!ret) {
        return false;
    }
    for (const SkillUri &skillUri : uris) {
        for (const auto &mimeType : mimeTypes) {
            if ((MatchUri(uriString, skillUri) ||
                (skillUri.scheme.empty() && uriString.find(SCHEME_SEPARATOR) == std::string::npos)) &&
                MatchType(mimeType, skillUri.type)) {
                return true;
            }
        }
    }
    return false;
}

bool SkillUtils::MatchMimeType(const std::string & uriString, size_t &matchUriIndex) const
{
    std::vector<std::string> mimeTypes;
    bool ret = GetMimeTypeByUri(uriString, mimeTypes);
    if (!ret) {
        return false;
    }
    for (size_t uriIndex = 0; uriIndex < uris.size(); ++uriIndex) {
        const SkillUri &skillUri = uris[uriIndex];
        for (const auto &mimeType : mimeTypes) {
            if ((MatchUri(uriString, skillUri) ||
                (skillUri.scheme.empty() && uriString.find(SCHEME_SEPARATOR) == std::string::npos)) &&
                MatchType(mimeType, skillUri.type)) {
                matchUriIndex = uriIndex;
                return true;
            }
        }
    }
    return false;
}

bool SkillUtils::StartsWith(const std::string &sourceString, const std::string &targetPrefix) const
{
    return sourceString.rfind(targetPrefix, 0) == 0;
}

std::string SkillUtils::GetOptParamUri(const std::string &uriString) const
{
    std::size_t pos = uriString.rfind(PARAM_SEPARATOR);
    if (pos == std::string::npos) {
        return uriString;
    }
    return uriString.substr(0, pos);
}

bool SkillUtils::GetMimeTypeByUri(const std::string &uri, std::vector<std::string> &mimeTypes) const
{
    std::string suffix;
    if (!GetUriSuffix(uri, suffix)) {
        APP_LOGE("Get suffix failed, uri is %{public}s", uri.c_str());
        return false;
    }

    auto range = mimeTypeMap_.equal_range(suffix);
    for (auto it = range.first; it != range.second; ++it) {
        mimeTypes.push_back(it->second);
    }
    if (mimeTypes.empty()) {
        APP_LOGE("Suffix %{public}s has no corresponding type", suffix.c_str());
        return false;
    }
    return true;
}

bool SkillUtils::GetUriSuffix(const std::string &uri, std::string &suffix) const
{
    auto suffixIndex = uri.rfind('.');
    if (suffixIndex == std::string::npos) {
        APP_LOGE("Get suffix failed, uri is %{public}s", uri.c_str());
        return false;
    }
    suffix = uri.substr(suffixIndex + 1);
    std::transform(suffix.begin(), suffix.end(), suffix.begin(),
                [](unsigned char c) { return std::tolower(c); });
    return true;
}

}
}