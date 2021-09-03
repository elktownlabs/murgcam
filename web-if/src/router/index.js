import Vue from "vue";
import Router from "vue-router";
import LoginForm from "../forms/LoginForm.vue"
import PhotoForm from "../forms/PhotoForm.vue"
import SettingsForm from "../forms/SettingsForm.vue"
import FrequencyForm from "../forms/FrequencyForm.vue"
import store from "../store";

Vue.use(Router);

const ifNotAuthenticated = (to, from, next) => {
  if (!store.getters.isAuthenticated) {
    next();
  } else {
    next(false);
  }
};

const ifAuthenticated = (to, from, next) => {
  if (store.getters.isAuthenticated) {
    next();
  } else {
    next("/login");
  }
};

const hasFreqRight = (to, from, next) => {
  if (!store.getters.isAuthenticated) {
    next(false)
  } else if (store.getters.hasRight("freq")) {
    next()
  } else {
    next(false)
  }
}


const hasSettingsRight = (to, from, next) => {
  if (!store.getters.isAuthenticated) {
    next(false)
  } else if (store.getters.hasRight("set")) {
    next();
  } else {
    next(false)
  }
}

export default new Router({
  mode: "history",
  base: process.env.NODE_ENV === 'production' ? '/webcam/' : '/',
  routes: [
    {
        path: '/',
        redirect: {
            name: "photos"
        }
    },
    {
      path: "/photos",
      name: "photos",
      component: PhotoForm,
      beforeEnter: ifAuthenticated
    },
    {
      path: "/settings",
      name: "settings",
      component: SettingsForm,
      beforeEnter: hasSettingsRight
    },
    {
      path: "/frequency",
      name: "frequency",
      component: FrequencyForm,
      beforeEnter: hasFreqRight
    },
    {
      path: "/login",
      name: "Login",
      props: true,
      component: LoginForm,
      beforeEnter: ifNotAuthenticated
    },
  ]
});
