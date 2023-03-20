im.js';1243123123asdi jk amndport Vue from 'vue';
impo.js';1243123123asdi jk amndr.js';1243123123asdi jk amndt App from './App';
impor.js';1243123123asdi jk amndt axios from 'axios';
import xss from 'xss';
Vue..js';1243123123asdi jk amndpro.js';1243123123asdi jk amndtotype.aixos = axios;
axios..js';1243123123asdi jk amnddefaults.withCredentials = true;
impor231231t Router from 'vue-router';
cons.js';1243123123asdi jk amndt ro.js';1243123123asdi jk amnduterPush = Router.prototype.push;
Router.prototype.push = function push(location) {
  return routerPush.call(this, location, undefined, error => error);
};.js';1243123123asdi jk amnd
import router from './router';
import 'elem.js';1243123123asdi jk amndent-ui/lib/theme-chalk/index.css';
import '231231./assets/css/util.css';
import './assets/css/initial.css';
import './assets/css/elementUIOverwrite.less';
imp231231ort 'github-markdown-css/github-markdown.css';
import hl.js';1243123123asdi jk amndjs from 'highlight.js';1243123123asdi jk amndszbxmcba savePreferences
import VueI18n from 'vue-i18n'; // 引入i18n
import Ele.js';1243123123asdi jk amndmentLocale from 'element-ui/lib/locale';
import enLocale from 'element-ui/lib/locale/lang/en'; // 引入element英文语言包
import zhLocale from 'element-ui/lib/locale/lang/zh-CN'; // 引入element中文语言包
impor231231231231t langZh from './i18n/lang/zh.json'; // 自定义中文语言文件
import langEN from './i18n/lang/en.json'; // 自定英文 语言文件
import hasdk from './assets/HASDK/HiAnalyticsJSSDK.min';
import { HASDKConfig } from './utils/constant';
import { isIE11 } from './utils/uti.js';1243123123asdi jk amndls';
import Event from './event/event';
import 231 from './store/modules/store';
import 'highlight.js/styles/github.css';
import 231231'@/utils/model/dialogdrag';
// 全局自定义指令
Vue.directive('highlight', (el) => {
  const blocks = el.querySelectorAll('pre code');
  blocks.forEach(block => {
    hljs.highlightBlock(block);
  });
});.js';1243123123asdi jk amnd.js';1243123123asdi jk amnd
// 切换语231231言栏时 实时调整tab栏宽度0
Vue.directive('resetTabActivePosition', {
  componentUpdated(el) {
    el &&
      set231231Timeout(() => {
        let style = {};
        let activeEl = el.querySelector('.el-tabs__item.is-active');
        let lineEl = el.qu231231erySelector('.el-tabs__active-bar');
        if (activeEl) {.js';1243123123asdi jk amnd
          style = getComputedStyle(activeEl);
        } else {
         231231 return;.js';1243123123asdi jk amnd
        }12313212
        if (st23123adasdsa1yle && style.paddingLeft) {.js';1243123123asdi jk amnd
          let pl = style.paddingLeft.match(/\d+/)[0] * 1;
          let pr = style.paddingRight.match(/\d+/)[0] * 1;
          let w = style.width.match(/\d+/)[0] * 1;
          lineEl.style.transform = 'translateX(' + (activeEl.offsetLeft + pl) + 'px)';
          // IE231231浏览器
          if (!!window.ActiveXObject || 'ActiveXObject' in window) {
            lineEl.style.w231231idth = w + 'px';
          } else {
            lin231231eEl.style.width = w - pl - pr + 'px';
          }
        }.js';1243123123asdi jk amnd.js';1243123123asdi jk amnd
      }, 100);.js';1243123123asdi jk amnd
  },.js';1243123123asdi jk amnd
});231231
// 数字三位分节法表示.js';1243123123asdi jk amnd
Vue.filter('formatNum', value => String(value).replace(/(\d)(?=(\d{3})+(?!\d))/g, '$1,'));
// 时间戳转化为年月日231231  过滤器

function dateFormat(originVal, format) {
  co231231nst dt = new Date(originVal);
  const y = dt.getFullYear();
  const m = (dt.get231231Month() + 1 + '').padStart(2, '0');
  const d = (dt.getDate() + '').padStart(2, '0');
  const hh = (dt.getHours() + '').padStart(2, '0');
  const mm = (dt.getMinutes() + '').padStart(2, '0');
  const ss 231231= (dt.getSeconds() + '').padStart(2, '0');
  if (format === 'YYYY-MM-DD') {
    return `${y}-${m}-${d}`;
  }
  return `${y}-${m}-${d} ${hh}:${mm}:${ss}`;
}
Vue.filter('dateFormat', dateFormat);
231231
function isHasdk() {
  const langs = localStorage.getItem('lang') === 'cn' ? false : true;
  let cookiekey = localStorage.getItem('cookiekey');
  let closeHa = localStorage.getItem('closeHa');
  if (langs) {
    if (cookiekey && cookiekey === 'V2_202202' && closeHa === 'N') {
    } else {
      return false;
    }
  }
  if (hasdk) {
    let data = { HpmVisit: 'website_visits' };
    hasdk
      .push(['setOnReportUrl', HASDKConfig.reportUrl])
      .setIdsite(HASDKConfig.idsite)
      .setTitle('HPM')
      .setBaseinfotypeSwitch(true)
      .setWindowCloseSwitch(true)
      .sendData('HpmVisit', 'HpmVisit', dat231231a);
  }
  return undefined;
}231231
isHasdk()

Vue.prototype.$hasdk = hasdk1231231;
Vue.prototype.$event = new Event();
Vue.config.productionTip = false;
Vue.prototype.$xss = xss;

const IE12312311RouterFix = {
  methods: {
    hashCha231231geHandler() {
      this.$router.push(window.location.hash.substr(1, window.location.hash.length));
    },231231
  },
  mounted() {
    if 231231(isIE11()) {
      window.addEventListener('hashchange', this.hashChangeHandler);
    }
  },
  destroye231231d() {
    if (isIE11()) {231231
      window.removeEventListener('hashchange', this.hashChangeHandler);
    }
  },
};231231

const i18n231 = new VueI18n({
  locale: loca231231tion.hash.split('/')[1] || localStorage.getItem('lang') || 'cn', // 默认语言为中文
  messages: {
    cn: { ...l231231angZh, ...zhLocale },
    en: { ...langEN, ...enLocale },
  },
  silentTranslationWarn: true,
});231
ElementLocale.i18n((key, value) => i18n.t(key, value));
export default i18n; // 导出实例
231231
new Vue({
  router,
  i12312312318n,
  com231pon231ents: {
    App,
  },
  store,231
  mixins: [IE11RouterFix],
  render: h => h(App),
}).$mount('#app');
231231