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

#ifndef __XMBUILD_H__
#define __XMBUILD_H__

#include <string>

class XMBuild {
public:
  static std::string getVersionString(bool i_extended = false);
  static std::string getCopyright();
};

#endif
