/*=============================================================================
XMOTO

This file is part of XMOTO.

XMOTO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XMOTO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

#include "XMBuild.h"
#include "svn_version.h"
#include <sstream>

#include "common/XMBuildConfig.h"

std::string XMBuild::getVersionString(bool i_extended) {
  std::ostringstream v_version;

  v_version << BUILD_MAJORVERSION << "." << BUILD_MINORVERSION << "."
            << BUILD_PATCHVERSION;

  if (i_extended) {
    std::string v_svn = svn_version();

    if (std::string(BUILD_EXTRAINFO) != "") {
      v_version << " ";
      v_version << BUILD_EXTRAINFO;
    }

    if (std::string(BUILD_EXTRAINFO) != "" && v_svn != "") {
      v_version << " (svn " + v_svn + ")";
    }
  }

  return v_version.str();
}

std::string XMBuild::getCopyright() {
  return BUILD_COPYRIGHT;
}
