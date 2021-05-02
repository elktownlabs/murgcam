import Vue from 'vue'
import Vuex from 'vuex'
import VueRouter from 'vue-router'
import App from './App.vue'
import store from "./store"
import router from "./router";
import vuetify from './plugins/vuetify';
import 'roboto-fontface/css/roboto/roboto-fontface.css'
import '@mdi/font/css/materialdesignicons.css'

Vue.config.productionTip = false

Vue.use(VueRouter)
Vue.use(Vuex)

new Vue({
  vuetify,
  render: h => h(App),
  router: router,
  store: store,
}).$mount('#app')
