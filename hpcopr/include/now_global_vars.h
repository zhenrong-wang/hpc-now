/*
 * This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
 * The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
 * It is distributed under the license: GNU Public License - v2.0
 * Bug report: info@hpc-now.com
*/

#ifndef NOW_GLOBAL_VARS_H
#define NOW_GLOBAL_VARS_H

/*
 * URL_REPO_ROOT: the root location of the Terraform binary and Terraform providers. 
 * The structure of this location should be:
 * URL_REPO_ROOT --- terraform-win64 --- binary_with_correct_version
 *                |                   |- alicloud_provider.zip
 *                |                   |- aws_provider.zip
 *                |                   |- tencentcloud_provider.zip
 *                |
 *                |-- terraform-darwin --- similar to the structure above
 *                |-- terraform ---------- this refers to the GNU/Linux version
 *                                         for GNU/Linux, currently there is no suffix
 *
 */
extern char URL_REPO_ROOT[LOCATION_LENGTH];

/*
 * URL_CODE_ROOT: the root location of the Terraform IaC codes.
 * The structure of this location should be:
 * URL_CODE_ROOT --- tf-templates-alicloud
 *                |- tf-templates-qcloud
 *                |- tf-templates-aws
 */

extern char URL_CODE_ROOT[LOCATION_LENGTH];

/* URL_SHELL_SCRIPTS must be a public online address! */
extern char URL_SHELL_SCRIPTS[LOCATION_LENGTH];
/* URL_NOW_CRYPTO: the location points to a valid now-crypto.exe executable */
extern char URL_NOW_CRYPTO[LOCATION_LENGTH];

/*
 * If the location is an online location starting with http or https, *FLAG = 0
 * Otherwise, *FLAG = 1
 */
extern int REPO_LOC_FLAG;
extern int CODE_LOC_FLAG;
extern int NOW_CRYPTO_LOC_FLAG;

#endif